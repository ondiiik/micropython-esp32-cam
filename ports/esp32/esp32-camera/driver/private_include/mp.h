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

#ifdef __cplusplus
}
#endif


#define ESP_LOG(_type_, _tag_, ...) \
    do \
    { \
        mp_printf(&mp_plat_print, "[%s] %s:%d", _type_, _tag_, __LINE__); \
        mp_printf(&mp_plat_print, " :: " __VA_ARGS__); \
        mp_printf(&mp_plat_print, "\n"); \
    } \
    while (false)

#undef  ESP_LOGE
#define ESP_LOGE(_tag_, ...) ESP_LOG("!!", _tag_, __VA_ARGS__)

#undef  ESP_LOGW
#define ESP_LOGW(_tag_, ...) ESP_LOG("**", _tag_, __VA_ARGS__)

#undef  ESP_LOGI
#define ESP_LOGI(_tag_, ...) ESP_LOG("..", _tag_, __VA_ARGS__)

#undef  ESP_LOGD
#define ESP_LOGD(_tag_, ...) ESP_LOG("  ", _tag_, __VA_ARGS__)

#undef  ESP_LOGV
#define ESP_LOGV(_tag_, ...) ESP_LOG("  ", _tag_, __VA_ARGS__)

