/*
 * SPDX-FileCopyrightText: 2026
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Camera driver for GC2145 via DVP.
 * I2C shared with ES8311 on I2C0 (GPIO4/5).
 */

#include "camera.h"

#include <string.h>

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"

#include "boardconfig.h"

static const char *TAG = "esp_vision_camera";

static bool s_camera_inited = false;

static void xclk_init(void)
{
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = ESP_VISION_CAMERA_XCLK_LEDC_TIMER,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = ESP_VISION_CAMERA_XCLK_FREQ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer);

    ledc_channel_config_t channel = {
        .channel = ESP_VISION_CAMERA_XCLK_LEDC_CHANNEL,
        .duty = 128,
        .gpio_num = ESP_VISION_CAMERA_XCLK_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel = ESP_VISION_CAMERA_XCLK_LEDC_TIMER,
    };
    ledc_channel_config(&channel);
}

void esp_vision_camera_init0(void)
{
}

esp_err_t esp_vision_camera_init(void)
{
    if (s_camera_inited) {
        return ESP_OK;
    }

    xclk_init();
    vTaskDelay(pdMS_TO_TICKS(50));

    /* Configure DVP GPIO pins */
    gpio_config_t io_conf = {
        .pin_bit_mask = (BIT64(ESP_VISION_CAMERA_DVP_D0_PIN) |
                         BIT64(ESP_VISION_CAMERA_DVP_D1_PIN) |
                         BIT64(ESP_VISION_CAMERA_DVP_D2_PIN) |
                         BIT64(ESP_VISION_CAMERA_DVP_D3_PIN) |
                         BIT64(ESP_VISION_CAMERA_DVP_D4_PIN) |
                         BIT64(ESP_VISION_CAMERA_DVP_D5_PIN) |
                         BIT64(ESP_VISION_CAMERA_DVP_D6_PIN) |
                         BIT64(ESP_VISION_CAMERA_DVP_D7_PIN) |
                         BIT64(ESP_VISION_CAMERA_DVP_PCLK_PIN) |
                         BIT64(ESP_VISION_CAMERA_DVP_VSYNC_PIN) |
                         BIT64(ESP_VISION_CAMERA_DVP_HSYNC_PIN)),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    s_camera_inited = true;
    ESP_LOGI(TAG, "Camera GPIO initialized. Actual camera init delegated to ESP-IDF esp_video pipeline.");
    return ESP_OK;
}

void esp_vision_camera_deinit(void)
{
    s_camera_inited = false;
}

bool esp_vision_camera_is_ready(void)
{
    return s_camera_inited;
}

esp_err_t esp_vision_camera_set_pixformat(uint32_t pixfmt)
{
    (void)pixfmt;
    if (!s_camera_inited) return ESP_ERR_INVALID_STATE;
    return ESP_OK;
}

uint32_t esp_vision_camera_get_pixformat(void)
{
    return PIXFORMAT_RGB565;
}

esp_err_t esp_vision_camera_set_framesize(esp_vision_camera_framesize_t framesize)
{
    (void)framesize;
    if (!s_camera_inited) return ESP_ERR_INVALID_STATE;
    return ESP_OK;
}

esp_err_t esp_vision_camera_get_framesize_dimensions(esp_vision_camera_framesize_t framesize,
                                                     uint32_t *width,
                                                     uint32_t *height)
{
    if ((width == NULL) || (height == NULL)) {
        return ESP_ERR_INVALID_ARG;
    }

    switch (framesize) {
    case ESP_VISION_CAMERA_FRAMESIZE_QQVGA:
        *width = 160; *height = 120; return ESP_OK;
    case ESP_VISION_CAMERA_FRAMESIZE_QVGA:
        *width = 320; *height = 240; return ESP_OK;
    case ESP_VISION_CAMERA_FRAMESIZE_VGA:
        *width = 640; *height = 480; return ESP_OK;
    default:
        return ESP_ERR_NOT_SUPPORTED;
    }
}

uint32_t esp_vision_camera_get_width(void) { return 640; }
uint32_t esp_vision_camera_get_height(void) { return 480; }

esp_err_t esp_vision_camera_set_hmirror(bool enable) { (void)enable; return ESP_ERR_NOT_SUPPORTED; }
bool esp_vision_camera_get_hmirror(void) { return false; }
esp_err_t esp_vision_camera_set_vflip(bool enable) { (void)enable; return ESP_ERR_NOT_SUPPORTED; }
bool esp_vision_camera_get_vflip(void) { return false; }

uint32_t esp_vision_camera_get_sensor_id(void) { return 0x2145; }

size_t esp_vision_camera_frame_size(void)
{
    return 640 * 480 * 2;
}

esp_err_t esp_vision_camera_capture(uint8_t *pixels, size_t pixels_size)
{
    (void)pixels; (void)pixels_size;
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t esp_vision_camera_snapshot(image_t *img, uint8_t *pixels, size_t pixels_size)
{
    (void)img; (void)pixels; (void)pixels_size;
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t esp_vision_camera_get_status(esp_vision_camera_status_t *status)
{
    if (status == NULL) return ESP_ERR_INVALID_ARG;
    memset(status, 0, sizeof(*status));
    return ESP_OK;
}
