/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct image image_t;

void esp_vision_preview_init0(void);
void esp_vision_preview_deinit(void);

esp_err_t esp_vision_preview_flush(const image_t *img);

#ifdef __cplusplus
}
#endif
