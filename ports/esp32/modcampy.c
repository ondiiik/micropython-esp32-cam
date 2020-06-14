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
    
    .jpeg_quality = 10, //0-63 lower number means higher quality
    .fb_count     = 1   //if more than one, i2s runs in continuous mode. Use only with JPEG
};



#include "esp_system.h"
#include "esp_spi_flash.h"


MP_FN_0(campy, init)
{
    esp_err_t err = esp_camera_init(&camera_config);
    
    if (err != ESP_OK)
    {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT("Camera Init Failed"));
    }
    
    return mp_const_none;
}


MP_FN_0(campy, deinit)
{
    esp_err_t err = esp_camera_deinit();
    
    if (err != ESP_OK)
    {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT("Camera deinit Failed"));
    }
    
    return mp_const_true;
}


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


STATIC void MP_NAMESPACE3(campy, FrameBuffer, __property__)(mp_obj_t  aSelf,
                                                            qstr      aAttribute,
                                                            mp_obj_t* aDestination)
{
    struct campy_FrameBuffer* self = MP_OBJ_TO_PTR(aSelf);
    
    switch (aAttribute)
    {
        case MP_QSTR_data:
            {
                mp_obj_str_t* bytes = m_new_obj(mp_obj_str_t);
                
                bytes->base.type = &mp_type_bytes;
                bytes->data      = (const byte*)self->fb.buf;
                bytes->len       = self->fb.len;
                bytes->hash      = qstr_compute_hash(bytes->data, bytes->len);
                
                *aDestination = bytes;
                break;
            }
            
            
        case MP_QSTR_width:
            {
                *aDestination = MP_OBJ_NEW_SMALL_INT(self->fb.width);
                break;
            }
            
            
        case MP_QSTR_height:
            {
                *aDestination = MP_OBJ_NEW_SMALL_INT(self->fb.height);
                break;
            }
    }
}


MP_CLASS_BEGIN(campy, FrameBuffer)
MP_CLASS_END


MP_FN_0(campy,capture)
{
    //acquire a frame
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
    
//    return the frame buffer back to the driver for reuse
    esp_camera_fb_return(fb);
    
    return frame;
}


MP_MODULE_BEGIN( campy)
MP_MEMBER(       campy, init)
MP_MEMBER(       campy, deinit)
MP_MEMBER(       campy, capture)
MP_MEMBER(       campy, FrameBuffer)
MP_MODULE_END(   campy)
