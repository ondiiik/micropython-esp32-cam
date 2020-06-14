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
 * @brief   Frame buffr type
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
