/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sdcard.h"

void esp_vision_board_sdcard_init0(void)
{
}

bool esp_vision_board_sdcard_is_present(void)
{
    return false;
}

esp_err_t esp_vision_board_sdcard_preinit_host(sdmmc_host_t *host, int slot)
{
    (void)host;
    (void)slot;
    return ESP_ERR_NOT_FOUND;
}

void esp_vision_board_sdcard_deinit_host(sdmmc_host_t *host, int slot)
{
    (void)host;
    (void)slot;
}
