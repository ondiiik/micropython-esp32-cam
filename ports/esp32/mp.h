/*
 * mp.h
 *
 *  Created on: Jun 12, 2020
 *      Author: ondiiik
 */
#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#include "py/mpconfig.h"

#include "py/misc.h"
#include "py/mpprint.h"
#include "py/runtime.h"

#include "modcampy.h"
#include "mp_namespace.h"

#ifdef __cplusplus
}
#endif


#define MP_LOG(_type_, _tag_, ...) \
    do \
    { \
        mp_printf(&mp_plat_print, "[%s] %s:%d", _type_, _tag_, __LINE__); \
        mp_printf(&mp_plat_print, " :: " __VA_ARGS__); \
        mp_printf(&mp_plat_print, "\n"); \
    } \
    while (false)

#define MP_LOGE(_tag_, ...) MP_LOG("!!", _tag_, __VA_ARGS__)
#define MP_LOGW(_tag_, ...) MP_LOG("**", _tag_, __VA_ARGS__)
#define MP_LOGI(_tag_, ...) MP_LOG("..", _tag_, __VA_ARGS__)
#define MP_LOGD(_tag_, ...) /*MP_LOG("  ", _tag_, __VA_ARGS__)*/
#define MP_LOGV(_tag_, ...) /*MP_LOG("  ", _tag_, __VA_ARGS__)*/

