#include "modcampy.h"

#include "esp_camera.h"
#include "esp_log.h"



#include "py/binary.h"
#include "py/mpprint.h"
#include "py/nlr.h"
#include "py/objstr.h"
#include "py/runtime.h"

#include "mp.h"
#include <stdarg.h>


/**
 * @brief   Active camera singleton pointer
 */
static struct campy_Camera* _activeCamera;


#define TAG "camera"


//WROVER-KIT PIN Map
#define CAM_PIN_PWDN    32 //power down is not used
#define CAM_PIN_RESET   -1 //software reset will be performed
#define CAM_PIN_XCLK     0
#define CAM_PIN_SIOD    26 // SDA
#define CAM_PIN_SIOC    27 // SCL

#define CAM_PIN_D7      35
#define CAM_PIN_D6      34
#define CAM_PIN_D5      39
#define CAM_PIN_D4      36
#define CAM_PIN_D3      21
#define CAM_PIN_D2      19
#define CAM_PIN_D1      18
#define CAM_PIN_D0       5
#define CAM_PIN_VSYNC   25
#define CAM_PIN_HREF    23
#define CAM_PIN_PCLK    22


static camera_config_t camera_config =
{
    .pin_pwdn     = CAM_PIN_PWDN,
    .pin_reset    = CAM_PIN_RESET,
    .pin_xclk     = CAM_PIN_XCLK,
    .pin_sscb_sda = CAM_PIN_SIOD,
    .pin_sscb_scl = CAM_PIN_SIOC,
    
    .pin_d7       = CAM_PIN_D7,
    .pin_d6       = CAM_PIN_D6,
    .pin_d5       = CAM_PIN_D5,
    .pin_d4       = CAM_PIN_D4,
    .pin_d3       = CAM_PIN_D3,
    .pin_d2       = CAM_PIN_D2,
    .pin_d1       = CAM_PIN_D1,
    .pin_d0       = CAM_PIN_D0,
    .pin_vsync    = CAM_PIN_VSYNC,
    .pin_href     = CAM_PIN_HREF,
    .pin_pclk     = CAM_PIN_PCLK,
    
    //XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    .xclk_freq_hz = 20000000,
    .ledc_timer   = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    
    .pixel_format = PIXFORMAT_JPEG,//YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size   = FRAMESIZE_UXGA,//QQVGA-UXGA Do not use sizes above QVGA when not JPEG
    
    .jpeg_quality = 4,  //0-63 lower number means higher quality
    .fb_count     = 1   //if more than one, i2s runs in continuous mode. Use only with JPEG
};



#include "esp_system.h"
#include "esp_spi_flash.h"




/*
 *******************************************************************************
 * campy.FrameBuffer class
 *******************************************************************************
 */
STATIC mp_obj_t MP_NAMESPACE3(campy, FrameBuffer, __init__)(const mp_obj_type_t* aType,
                                                            size_t               aArgsCnt,
                                                            size_t               aKw,
                                                            const mp_obj_t*      aArgs)
{
    mp_raise_NotImplementedError(MP_ERROR_TEXT("Object shall be created by <campy.Camera> class only"));
    return mp_const_none;
}


STATIC void MP_NAMESPACE3(campy, FrameBuffer, __str__)(const mp_print_t* aPrint,
                                                       mp_obj_t          aSelf,
                                                       mp_print_kind_t   aKind)
{
    struct campy_FrameBuffer* self = MP_OBJ_TO_PTR(aSelf);
    mp_printf(aPrint,
              "FrameBuffer(JPEG:%dx%d:%d)",
              self->fb.width,
              self->fb.height,
              self->fb.len);
}


STATIC mp_obj_t MP_NAMESPACE2(campy__FrameBuffer, data)(mp_obj_t  aSelf)
{
    struct campy_FrameBuffer* self  = MP_OBJ_TO_PTR(aSelf);
    mp_obj_str_t*             bytes = m_new_obj(mp_obj_str_t);
    
    bytes->base.type    = &mp_type_bytes;
    bytes->data         = (const byte*)self->fb.buf;
    bytes->len          = self->fb.len;
    bytes->hash         = qstr_compute_hash(bytes->data, bytes->len);
    
    return bytes;
}


STATIC mp_obj_t MP_NAMESPACE2(campy__FrameBuffer, width)(mp_obj_t  aSelf)
{
    struct campy_FrameBuffer* self  = MP_OBJ_TO_PTR(aSelf);
    return MP_OBJ_NEW_SMALL_INT(self->fb.width);
}


STATIC mp_obj_t MP_NAMESPACE2(campy__FrameBuffer, height)(mp_obj_t  aSelf)
{
    struct campy_FrameBuffer* self  = MP_OBJ_TO_PTR(aSelf);
    return MP_OBJ_NEW_SMALL_INT(self->fb.height);
}


STATIC void MP_NAMESPACE3(campy, FrameBuffer, __attr__)(mp_obj_t  aSelf,
                                                        qstr      aAttribute,
                                                        mp_obj_t* aDestination)
{
    switch (aAttribute)
    {
        MP_ATTR_PROPERTY(campy__FrameBuffer, data,   aSelf);
        MP_ATTR_PROPERTY(campy__FrameBuffer, width,  aSelf);
        MP_ATTR_PROPERTY(campy__FrameBuffer, height, aSelf);
    }
}


MP_CLASS(campy, FrameBuffer)




/*
 *******************************************************************************
 * campy.Camera class
 *******************************************************************************
 */
STATIC mp_obj_t MP_NAMESPACE3(campy, Camera, __init__)(const mp_obj_type_t* aType,
                                                       size_t               aArgsCnt,
                                                       size_t               aKw,
                                                       const mp_obj_t*      aArgs)
{
    /*
     * If we initializing after previous camera initialization, then we shall
     * call de-init first bo stop previous camera instance.
     */
    if (NULL != _activeCamera)
    {
        if (ESP_OK != esp_camera_deinit())
        {
            mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT("Camera re-initialization failed"));
        }
        
        _activeCamera = NULL;
    }
    
    /*
     * Now allocate new camera object and initialize it
     */
    struct campy_Camera*   activeCamera = campy_Camera_new(&camera_config);
    campy_Camera_init(     activeCamera);
    _activeCamera        = activeCamera;
    return MP_OBJ_FROM_PTR(activeCamera);
}


STATIC void MP_NAMESPACE3(campy, Camera, __str__)(const mp_print_t* aPrint,
                                                  mp_obj_t          aSelf,
                                                  mp_print_kind_t   aKind)
{
    struct campy_Camera* self = MP_OBJ_TO_PTR(aSelf);
    
    if (_activeCamera != self)
    {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT("This camera was replaced by another active camera"));
    }
    
    mp_printf(aPrint, "Camera(%s)", self->model);
}


MP_FN_1(campy__Camera, capture, aSelf)
{
    struct campy_Camera* self = MP_OBJ_TO_PTR(aSelf);
    
    if (_activeCamera != self)
    {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT("This camera was replaced by another active camera"));
    }
    
    
    camera_fb_t* fb = esp_camera_fb_get();
    
    if (!fb)
    {
        ESP_LOGE(TAG, "Camera Capture Failed");
        return mp_const_false;
    }
    
    mp_obj_t frame = campy_FrameBuffer_new(fb->width,
                                           fb->height,
                                           fb->format,
                                           fb->buf,
                                           fb->len);
    
    esp_camera_fb_return(fb);
    
    return frame;
}


STATIC mp_obj_t MP_NAMESPACE2(campy__Camera, model)(mp_obj_t  aSelf)
{
    struct campy_Camera* self = MP_OBJ_TO_PTR(aSelf);
    
    if (_activeCamera != self)
    {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT("This camera was replaced by another active camera"));
    }
    
    mp_obj_str_t* str = m_new_obj(mp_obj_str_t);
    
    str->base.type    = &mp_type_str;
    str->data         = (const byte*)self->model;
    str->len          = strlen(self->model);
    str->hash         = qstr_compute_hash(str->data, str->len);
    
    return str;
}



STATIC void MP_NAMESPACE3(campy, Camera, __attr__)(mp_obj_t  aSelf,
                                                   qstr      aAttribute,
                                                   mp_obj_t* aDestination)
{
    switch (aAttribute)
    {
        MP_ATTR_PROPERTY(campy__Camera, model,   aSelf);
        MP_ATTR_METHOD(  campy__Camera, capture, aSelf);
    }
}


MP_CLASS(campy, Camera)




/*
 *******************************************************************************
 * campy module
 *******************************************************************************
 */
MP_MODULE_BEGIN( campy         )
MP_MEMBER(       campy, Camera )
MP_MODULE_END(   campy         )
