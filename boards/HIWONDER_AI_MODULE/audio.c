/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * ES8311 audio codec driver for HIWONDER_AI_MODULE.
 * I2C on I2C0 (shared with camera and touch), I2S for audio data.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "driver/i2c.h"
#include "driver/i2s_std.h"
#include "esp_err.h"
#include "esp_log.h"

#include "boardconfig.h"

static const char *TAG = "esp_vision_audio";

/* ES8311 I2C address */
#define ES8311_ADDR                 0x18

/* ES8311 registers */
#define ES8311_RESET_REG            0x00
#define ES8311_CLOCK_MANAGER_REG    0x01
#define ES8311_CLOCK_DIV_REG        0x02
#define ES8311_ADC_DAC_REG          0x04
#define ES8311_SEL_ADC_REG          0x08
#define ES8311_ADC_VOL_REG          0x0B
#define ES8311_DAC_VOL_REG          0x0C
#define ES8311_SEL_DAC_REG          0x0D
#define ES8311_GPIO_REG             0x0F
#define ES8311_DAC_CTRL_REG         0x11
#define ES8311_ADC_CTRL_REG         0x12

/* ES8311 register values */
#define ES8311_RESET                0x1F
#define ES8311_CLOCK_ON             0x0F
#define ES8311_CLOCK_OFF            0x00
#define ES8311_ADC_DAC_ON           0x15
#define ES8311_ADC_DAC_OFF          0x00
#define ES8311_MCLK_DIV_256         0x00
#define ES8311_ADC_INPUT_MIC        0x62
#define ES8311_DAC_OUTPUT_LINE      0x04
#define ES8311_GPIO_DAC_PDN_N       0x02

static bool s_audio_inited = false;
static i2s_chan_handle_t s_tx_handle = NULL;

static esp_err_t es8311_write_reg(uint8_t reg, uint8_t val)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ES8311_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, val, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return ret;
}

static esp_err_t es8311_read_reg(uint8_t reg, uint8_t *val)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ES8311_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ES8311_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, val, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t esp_vision_audio_init(void)
{
    if (s_audio_inited) {
        return ESP_OK;
    }

    /* Reset ES8311 */
    esp_err_t ret = es8311_write_reg(ES8311_RESET_REG, ES8311_RESET);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to reset ES8311: %s", esp_err_to_name(ret));
        return ret;
    }
    vTaskDelay(pdMS_TO_TICKS(50));

    /* Clock management: enable all clocks */
    es8311_write_reg(ES8311_CLOCK_MANAGER_REG, ES8311_CLOCK_ON);

    /* MCLK divider */
    es8311_write_reg(ES8311_CLOCK_DIV_REG, ES8311_MCLK_DIV_256);

    /* ADC/DAC power on */
    es8311_write_reg(ES8311_ADC_DAC_REG, ES8311_ADC_DAC_ON);

    /* Select ADC input: microphone */
    es8311_write_reg(ES8311_SEL_ADC_REG, ES8311_ADC_INPUT_MIC);

    /* ADC volume */
    es8311_write_reg(ES8311_ADC_VOL_REG, 0x30);

    /* DAC volume */
    es8311_write_reg(ES8311_DAC_VOL_REG, 0x30);

    /* Select DAC output: line out */
    es8311_write_reg(ES8311_SEL_DAC_REG, ES8311_DAC_OUTPUT_LINE);

    /* GPIO: DAC power down not */
    es8311_write_reg(ES8311_GPIO_REG, ES8311_GPIO_DAC_PDN_N);

    /* DAC control */
    es8311_write_reg(ES8311_DAC_CTRL_REG, 0x08);

    /* ADC control */
    es8311_write_reg(ES8311_ADC_CTRL_REG, 0x08);

    ESP_LOGI(TAG, "ES8311 initialized successfully");
    s_audio_inited = true;
    return ESP_OK;
}

void esp_vision_audio_deinit(void)
{
    if (s_tx_handle) {
        i2s_channel_disable(s_tx_handle);
        i2s_del_channel(s_tx_handle);
        s_tx_handle = NULL;
    }
    s_audio_inited = false;
}

bool esp_vision_audio_is_ready(void)
{
    return s_audio_inited;
}
