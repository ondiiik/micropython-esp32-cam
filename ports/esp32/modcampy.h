/**
 * @brief   Camera to python bindings
 * 
 * With python there is slightly different allocating strategy approach.
 * Free of memory is done by garbage collector. With this concept is better
 * to give driver allocated frame buffer and let it to fill it in and return.
 * When all memory is used, garbage collector is activated and free all
 * unused frame buffers automatically.
 * 
 * @file    esp_campy.h
 * @since   Jun 13, 2020
 * @author  OSi (Sienczak)
 */
#pragma once


#include "esp_camera.h"
#include "mp_namespace.h"
#include "py/obj.h"
#include <stddef.h>


/**
 * @brief   Frame buffer type
 */
struct campy_FrameBuffer
{
    mp_obj_base_t base;   /**< @brief Type object */
    camera_fb_t   fb;     /**< @brief Frame buffer structure */
};


/**
 * @brief   Allocates new frame buffer
 * 
 * Allocates new frame buffer according to current camera configuration
 * 
 * @param aWidth    Frame width
 * @param aHeight   Frame height
 * @param aData     Frame data
 * @param aLen      Frame data length
 * 
 * @return  Allocated frame buffer
 */
extern struct campy_FrameBuffer* campy_FrameBuffer_new(unsigned    aWidth,
                                                       unsigned    aHeight,
                                                       pixformat_t aPixFirmat,
                                                       const byte* aData,
                                                       size_t      aLen);


/**
 * @brief   Python framebuffer structure
 */
extern const mp_obj_type_t MP_NAMESPACE2(campy, FrameBuffer);


/**
 * @brief   Camera type
 */
struct campy_Camera
{
    mp_obj_base_t   base;       /**< @brief Type object */
    camera_config_t config;     /**< @brief Frame buffer structure */
    char            model[7];   /**< @brief Camera model string */
};


/**
 * @brief   Allocates new camera
 * 
 * Allocates new camera and deactivates old one
 * 
 * @param aConfig   New camera configuration
 * 
 * @return  Allocated camera object
 */
extern struct campy_Camera* campy_Camera_new(const camera_config_t* aConfig);


/**
 * @brief Initialize the camera driver
 *
 * @note call camera_probe before calling this function
 *
 * This function detects and configures camera over I2C interface,
 * allocates framebuffer and DMA buffers,
 * initializes parallel I2S input, and sets up DMA descriptors.
 *
 * Currently this function can only be called once and there is
 * no way to de-initialize this module.
 *
 * @param camera  Camera instance including suitable parameters
 */
void campy_Camera_init(struct campy_Camera* camera);


/**
 * @brief   Python camera structure
 */
extern const mp_obj_type_t MP_NAMESPACE2(campy, Camera);
