/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * ES8311 audio codec driver for HIWONDER_AI_MODULE.
 * I2C on I2C0 (shared with camera and touch), I2S for audio data.
 *
 * IMPORTANT: ES8311 uses MCLK (GPIO45) as its clock source.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/i2s_std.h"
#include "esp_err.h"
#include "esp_log.h"

#include "boardconfig.h"

static const char *TAG = "esp_vision_audio";

#define ES8311_ADDR                    0x18

#define ES8311_RESET_REG               0x00
#define ES8311_CLOCK_MANAGER_REG       0x01
#define ES8311_CLOCK_DIV_REG           0x02
#define ES8311_ADC_DAC_REG             0x04
#define ES8311_SEL_ADC_REG             0x08
#define ES8311_ADC_VOL_REG             0x0B
#define ES8311_DAC_VOL_REG             0x0C
#define ES8311_SEL_DAC_REG             0x0D
#define ES8311_GPIO_REG                0x0F
#define ES8311_DAC_CTRL_REG            0x11
#define ES8311_ADC_CTRL_REG            0x12

#define ES8311_RESET                   0x1F
/* Use MCLK as clock source (bit 6=0), all power on */
#define ES8311_CLOCK_MCLK_ON           0x0F
#define ES8311_CLOCK_OFF               0x00
#define ES8311_ADC_DAC_ON              0x15
#define ES8311_ADC_DAC_OFF             0x00
#define ES8311_MCLK_DIV_256            0x00
#define ES8311_ADC_INPUT_MIC           0x62
#define ES8311_DAC_OUTPUT_LINE         0x04
#define ES8311_GPIO_DAC_PDN_N          0x02

#define AUDIO_DEFAULT_SAMPLE_RATE      44100

static bool s_audio_initialized = false;
static bool s_i2c_installed = false;
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

esp_err_t esp_vision_audio_init(void)
{
    if (s_audio_initialized) {
        return ESP_OK;
    }

    /*
     * I2C0 may already be installed by camera (via SCCB).
     * Try driver_install FIRST; only call param_config if we actually install it.
     * This avoids calling param_config on an already-configured I2C bus.
     */
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 4,
        .scl_io_num = 5,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };

    esp_err_t ret = i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
    if (ret == ESP_ERR_INVALID_STATE) {
        /* Driver already installed by camera - just reuse it */
        ESP_LOGI(TAG, "I2C0 already installed (by camera), reusing");
    } else if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C0 install failed: %s", esp_err_to_name(ret));
        return ret;
    } else {
        /* We installed the driver, now configure it */
        i2c_param_config(I2C_NUM_0, &conf);
        s_i2c_installed = true;
        ESP_LOGI(TAG, "I2C0 installed and configured");
    }

    /* Reset ES8311 with retry */
    for (int i = 0; i < 3; i++) {
        ret = es8311_write_reg(ES8311_RESET_REG, ES8311_RESET);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "ES8311 reset OK");
            break;
        }
        ESP_LOGW(TAG, "ES8311 reset attempt %d failed: %s", i+1, esp_err_to_name(ret));
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ES8311 not responding on I2C0");
        return ret;
    }
    vTaskDelay(pdMS_TO_TICKS(50));

    /* Enable PA (SPK_EN=GPIO46) */
    gpio_set_direction(46, GPIO_MODE_OUTPUT);
    gpio_set_level(46, 1);
    ESP_LOGI(TAG, "PA enabled");

    /*
     * Configure ES8311 registers.
     * CRITICAL: Register 0x01 uses MCLK (GPIO45) as clock source (0x0F).
     * Bit 6 = 0: select MCLK as clock source.
     */
    es8311_write_reg(ES8311_CLOCK_MANAGER_REG, ES8311_CLOCK_MCLK_ON);
    es8311_write_reg(ES8311_CLOCK_DIV_REG, ES8311_MCLK_DIV_256);
    es8311_write_reg(ES8311_ADC_DAC_REG, ES8311_ADC_DAC_ON);
    es8311_write_reg(ES8311_SEL_ADC_REG, ES8311_ADC_INPUT_MIC);
    es8311_write_reg(ES8311_ADC_VOL_REG, 0x30);
    es8311_write_reg(ES8311_DAC_VOL_REG, 0x30);
    es8311_write_reg(ES8311_SEL_DAC_REG, ES8311_DAC_OUTPUT_LINE);
    es8311_write_reg(ES8311_GPIO_REG, ES8311_GPIO_DAC_PDN_N);
    es8311_write_reg(ES8311_DAC_CTRL_REG, 0x08);
    es8311_write_reg(ES8311_ADC_CTRL_REG, 0x08);

    /* Initialize I2S TX channel */
    i2s_chan_config_t chan_cfg = {
        .id = I2S_NUM_0,
        .role = I2S_ROLE_MASTER,
        .dma_desc_num = 8,
        .dma_frame_num = 256,
        .auto_clear_after_cb = true,
    };
    ret = i2s_new_channel(&chan_cfg, &s_tx_handle, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2S new channel: %s", esp_err_to_name(ret));
        return ret;
    }

    /* ES8311 uses MCLK (GPIO45) as clock source */
    i2s_std_config_t std_cfg = {
        .clk_cfg = { .sample_rate_hz = AUDIO_DEFAULT_SAMPLE_RATE,
            .clk_src = I2S_CLK_SRC_DEFAULT, .mclk_multiple = I2S_MCLK_MULTIPLE_256 },
        .slot_cfg = { .slot_mode = I2S_SLOT_MODE_STEREO,
            .slot_mask = I2S_STD_SLOT_BOTH, .ws_width = 32,
            .bit_shift = true, .left_align = true,
            .big_endian = false, .bit_order_lsb = false },
        .gpio_cfg = { .mclk = 45,
            .bclk = 39, .ws = 41, .dout = 42, .din = 40 },
    };
    ret = i2s_channel_init_std_mode(s_tx_handle, &std_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2S init std: %s", esp_err_to_name(ret));
        i2s_del_channel(s_tx_handle); s_tx_handle = NULL;
        return ret;
    }

    ret = i2s_channel_enable(s_tx_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2S enable: %s", esp_err_to_name(ret));
        i2s_del_channel(s_tx_handle); s_tx_handle = NULL;
        return ret;
    }

    ESP_LOGI(TAG, "ES8311 + I2S initialized (MCLK clock source)");
    s_audio_initialized = true;
    return ESP_OK;
}

void esp_vision_audio_deinit(void)
{
    if (s_tx_handle) {
        i2s_channel_disable(s_tx_handle);
        i2s_del_channel(s_tx_handle);
        s_tx_handle = NULL;
    }
    if (s_i2c_installed) {
        i2c_driver_delete(I2C_NUM_0);
        s_i2c_installed = false;
    }
    s_audio_initialized = false;
}

bool esp_vision_audio_is_ready(void) { return s_audio_initialized; }

int esp_vision_audio_play_pcm(const uint8_t *data, size_t len, uint32_t sample_rate)
{
    if (!s_audio_initialized || !s_tx_handle || !data || len == 0) return -1;
    if (sample_rate != 0) {
        i2s_channel_disable(s_tx_handle);
        i2s_std_clk_config_t clk_cfg = {
            .sample_rate_hz = sample_rate,
            .clk_src = I2S_CLK_SRC_DEFAULT,
            .mclk_multiple = I2S_MCLK_MULTIPLE_256,
        };
        i2s_channel_reconfig_std_clock(s_tx_handle, &clk_cfg);
        i2s_channel_enable(s_tx_handle);
    }
    size_t bytes_written = 0;
    esp_err_t ret = i2s_channel_write(s_tx_handle, data, len, &bytes_written, portMAX_DELAY);
    if (ret != ESP_OK) return -1;
    return (int)bytes_written;
}
