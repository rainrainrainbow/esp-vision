/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * FT5x06 capacitive touch controller driver for HIWONDER_AI_MODULE.
 * I2C on I2C0 (shared with camera and audio).
 */

#include <stdbool.h>
#include <stdint.h>

#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"

#include "boardconfig.h"

static const char *TAG = "esp_vision_touch";

/* FT5x06 I2C address */
#define FT5X06_ADDR                 0x38

/* FT5x06 registers */
#define FT5X06_DEVICE_MODE          0x00
#define FT5X06_TD_STATUS            0x02
#define FT5X06_TOUCH1_XH            0x03
#define FT5X06_TOUCH1_XL            0x04
#define FT5X06_TOUCH1_YH            0x05
#define FT5X06_TOUCH1_YL            0x06
#define FT5X06_TOUCH2_XH            0x09
#define FT5X06_TOUCH2_XL            0x0A
#define FT5X06_TOUCH2_YH            0x0B
#define FT5X06_TOUCH2_YL            0x0C
#define FT5X06_ID_G_THGROUP         0x80
#define FT5X06_ID_G_PERIODACTIVE    0x88
#define FT5X06_ID_G_CIPHER          0xA3

typedef struct {
    bool touched;
    uint16_t x;
    uint16_t y;
    uint8_t touch_id;
    uint8_t touch_count;
} ft5x06_touch_point_t;

static bool s_touch_inited = false;

static esp_err_t ft5x06_read_regs(uint8_t reg, uint8_t *data, size_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (FT5X06_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (FT5X06_ADDR << 1) | I2C_MASTER_READ, true);
    if (len > 1) {
        i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, data + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t esp_vision_touch_init(void)
{
    if (s_touch_inited) {
        return ESP_OK;
    }

    /* Verify FT5x06 presence by reading device mode */
    uint8_t mode = 0;
    esp_err_t ret = ft5x06_read_regs(FT5X06_DEVICE_MODE, &mode, 1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to detect FT5x06: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "FT5x06 detected, device mode: 0x%02x", mode);
    s_touch_inited = true;
    return ESP_OK;
}

void esp_vision_touch_deinit(void)
{
    s_touch_inited = false;
}

bool esp_vision_touch_is_ready(void)
{
    return s_touch_inited;
}

esp_err_t esp_vision_touch_read_point(uint16_t *x, uint16_t *y, bool *touched)
{
    if (!x || !y || !touched) {
        return ESP_ERR_INVALID_ARG;
    }

    *touched = false;

    if (!s_touch_inited) {
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t data[6] = {0};
    esp_err_t ret = ft5x06_read_regs(FT5X06_TD_STATUS, data, 6);
    if (ret != ESP_OK) {
        return ret;
    }

    uint8_t touch_count = data[0] & 0x0F;
    if (touch_count == 0 || touch_count > 2) {
        return ESP_OK;
    }

    /* Read first touch point */
    *x = ((uint16_t)(data[1] & 0x0F) << 8) | data[2];
    *y = ((uint16_t)(data[3] & 0x0F) << 8) | data[4];
    *touched = true;

    return ESP_OK;
}
