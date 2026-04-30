/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>

#include "py/obj.h"

#ifndef CMSIS_MCU_H
#define CMSIS_MCU_H "cmsis_compiler.h"
#endif

#ifndef NO_QSTR
#include "imlib.h"
#else
typedef struct image image_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern const mp_obj_type_t esp_vision_image_type;

mp_obj_t esp_vision_image_new(int32_t width,
                              int32_t height,
                              uint32_t pixfmt,
                              uint8_t *pixels,
                              mp_obj_t backing);
image_t *esp_vision_image_get_image(mp_obj_t obj);

#ifdef __cplusplus
}
#endif
