/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * No SD card slot on HIWONDER_AI_MODULE.
 */

#include "sdcard.h"
#include "boardconfig.h"

esp_err_t esp_vision_board_sdcard_init(void)
{
    return ESP_ERR_NOT_SUPPORTED;
}

void esp_vision_board_sdcard_deinit(void)
{
}
