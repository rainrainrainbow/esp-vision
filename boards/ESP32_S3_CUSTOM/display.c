/*
 * SPDX-FileCopyrightText: 2026
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * SPI LCD display driver for 320x240 ST7789 panel.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"

#include "boardconfig.h"

static bool s_backlight_inited;

void esp_vision_board_display_deinit_panel(esp_lcd_panel_io_handle_t io_handle,
                                           esp_lcd_panel_handle_t panel_handle);

static esp_err_t config_output_gpio(gpio_num_t gpio_num, int level)
{
    if (gpio_num == GPIO_NUM_NC || gpio_num < 0) return ESP_OK;
    gpio_config_t cfg = {
        .pin_bit_mask = BIT64(gpio_num),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t ret = gpio_config(&cfg);
    if (ret != ESP_OK) return ret;
    return gpio_set_level(gpio_num, level);
}

esp_err_t esp_vision_board_display_backlight_init(void)
{
    if (s_backlight_inited) return ESP_OK;
    esp_err_t ret = config_output_gpio(ESP_VISION_LCD_PIN_BL, 0);
    if (ret == ESP_OK) s_backlight_inited = true;
    return ret;
}

void esp_vision_board_display_set_backlight(uint32_t backlight)
{
    (void)config_output_gpio(ESP_VISION_LCD_PIN_BL, backlight > 0);
}

esp_err_t esp_vision_board_display_init_panel(uint32_t width,
                                              uint32_t height,
                                              esp_lcd_panel_io_handle_t *io_handle,
                                              esp_lcd_panel_handle_t *panel_handle)
{
    if ((width != ESP_VISION_LCD_WIDTH) || (height != ESP_VISION_LCD_HEIGHT) ||
        !io_handle || !panel_handle) return ESP_ERR_INVALID_ARG;

    *io_handle = NULL;
    *panel_handle = NULL;

    esp_err_t ret = config_output_gpio(ESP_VISION_LCD_PIN_BL, 0);
    if (ret != ESP_OK) return ret;

    spi_bus_config_t buscfg = {
        .sclk_io_num = ESP_VISION_LCD_PIN_CLK,
        .mosi_io_num = ESP_VISION_LCD_PIN_MOSI,
        .miso_io_num = GPIO_NUM_NC,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = width * height * 2 + 8,
    };
    ret = spi_bus_initialize(ESP_VISION_LCD_SPI_HOST_ID, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) return ret;

    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = ESP_VISION_LCD_PIN_DC,
        .cs_gpio_num = ESP_VISION_LCD_PIN_CS,
        .pclk_hz = ESP_VISION_LCD_SPI_CLOCK_HZ,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)ESP_VISION_LCD_SPI_HOST_ID, &io_config, io_handle);
    if (ret != ESP_OK) { spi_bus_free(ESP_VISION_LCD_SPI_HOST_ID); return ret; }

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = ESP_VISION_LCD_PIN_RST,
        .rgb_endian = LCD_RGB_ENDIAN_RGB,
        .bits_per_pixel = 16,
    };
    ret = esp_lcd_new_panel_st7789(*io_handle, &panel_config, panel_handle);
    if (ret != ESP_OK) {
        esp_lcd_panel_io_del(*io_handle);
        *io_handle = NULL;
        spi_bus_free(ESP_VISION_LCD_SPI_HOST_ID);
        return ret;
    }

    ret = esp_lcd_panel_reset(*panel_handle);
    if (ret == ESP_OK) ret = esp_lcd_panel_init(*panel_handle);
    if (ret == ESP_OK) ret = esp_lcd_panel_invert_color(*panel_handle, true);
    if (ret == ESP_OK) ret = esp_lcd_panel_swap_xy(*panel_handle, true);
    if (ret == ESP_OK) ret = esp_lcd_panel_mirror(*panel_handle, true, false);
    if (ret == ESP_OK) ret = esp_lcd_panel_disp_on_off(*panel_handle, true);
    if (ret == ESP_OK) {
        esp_vision_board_display_set_backlight(100);
    } else {
        esp_vision_board_display_deinit_panel(NULL, *panel_handle);
        *panel_handle = NULL;
    }
    return ret;
}

void esp_vision_board_display_deinit_panel(esp_lcd_panel_io_handle_t io_handle,
                                           esp_lcd_panel_handle_t panel_handle)
{
    (void)io_handle;
    if (panel_handle) {
        esp_lcd_panel_disp_on_off(panel_handle, false);
        esp_lcd_panel_del(panel_handle);
    }
    if (io_handle) esp_lcd_panel_io_del(io_handle);
    spi_bus_free(ESP_VISION_LCD_SPI_HOST_ID);
    esp_vision_board_display_set_backlight(0);
}
