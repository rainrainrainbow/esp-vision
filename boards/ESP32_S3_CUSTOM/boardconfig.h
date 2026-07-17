/*
 * SPDX-FileCopyrightText: 2026
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ESP_VISION_BOARD_CONFIG_H
#define ESP_VISION_BOARD_CONFIG_H

#define ESP_VISION_BOARD_ARCH                  "ESP32S3"
#define ESP_VISION_BOARD_TYPE                  "ESP32_S3_CUSTOM"
#define ESP_VISION_PORT_ESP32                       (1)

#define ESP_VISION_IMLIB_PROFILER_ENABLE            (0)
#define ESP_VISION_IMLIB_GPU_ENABLE                 (0)
#define ESP_VISION_IMLIB_JPEG_CODEC_ENABLE          (0)

#define ESP_VISION_CACHE_LINE_SIZE                  (32)
#define ESP_VISION_ALLOC_ALIGNMENT                  (ESP_VISION_CACHE_LINE_SIZE)
#define ESP_VISION_DMA_ALIGNMENT                    (ESP_VISION_CACHE_LINE_SIZE)

#define ESP_VISION_JPEG_QUALITY_LOW                 (60)
#define ESP_VISION_JPEG_QUALITY_HIGH                (60)
#define ESP_VISION_JPEG_QUALITY_THRESHOLD           (320 * 240 * 2)

/* Camera configuration - GC2145 via DVP */
#define ESP_VISION_CAMERA_SENSOR_ID                 (0x2145)
#define ESP_VISION_CAMERA_RAW_INPUT_WIDTH           (640)
#define ESP_VISION_CAMERA_RAW_INPUT_HEIGHT          (480)
#define ESP_VISION_CAMERA_ACTIVE_INPUT_WIDTH        (640)
#define ESP_VISION_CAMERA_ACTIVE_INPUT_HEIGHT       (480)
#define ESP_VISION_CAMERA_PPA_OUTPUT_QQVGA_WIDTH    (160)
#define ESP_VISION_CAMERA_PPA_OUTPUT_QQVGA_HEIGHT   (120)
#define ESP_VISION_CAMERA_PPA_OUTPUT_QVGA_WIDTH     (320)
#define ESP_VISION_CAMERA_PPA_OUTPUT_QVGA_HEIGHT    (240)
#define ESP_VISION_CAMERA_PPA_OUTPUT_VGA_WIDTH      (640)
#define ESP_VISION_CAMERA_PPA_OUTPUT_VGA_HEIGHT     (480)
#define ESP_VISION_CAMERA_BUFFER_COUNT              (3)

/* Camera I2C - shared with ES8311 on I2C0 */
#define ESP_VISION_CAMERA_SCCB_I2C_PORT             (0)
#define ESP_VISION_CAMERA_SCCB_I2C_SCL_PIN          (5)
#define ESP_VISION_CAMERA_SCCB_I2C_SDA_PIN          (4)
#define ESP_VISION_CAMERA_SENSOR_RESET_PIN          (-1)
#define ESP_VISION_CAMERA_SENSOR_PWDN_PIN           (-1)
#define ESP_VISION_CAMERA_SCCB_I2C_FREQ             (100000)
#define ESP_VISION_CAMERA_SCCB_INTERNAL_PULLUP      (1)

#define ESP_VISION_CAMERA_XCLK_PIN                  (15)
#define ESP_VISION_CAMERA_XCLK_FREQ                 (8000000)
#define ESP_VISION_CAMERA_XCLK_STABLE_MS            (50)
#define ESP_VISION_CAMERA_XCLK_LEDC_TIMER           (1)
#define ESP_VISION_CAMERA_XCLK_LEDC_CHANNEL         (0)

/* DVP pins */
#define ESP_VISION_CAMERA_DVP_PCLK_PIN              (13)
#define ESP_VISION_CAMERA_DVP_VSYNC_PIN             (6)
#define ESP_VISION_CAMERA_DVP_HSYNC_PIN             (7)
#define ESP_VISION_CAMERA_DVP_D0_PIN                (11)
#define ESP_VISION_CAMERA_DVP_D1_PIN                (9)
#define ESP_VISION_CAMERA_DVP_D2_PIN                (8)
#define ESP_VISION_CAMERA_DVP_D3_PIN                (10)
#define ESP_VISION_CAMERA_DVP_D4_PIN                (12)
#define ESP_VISION_CAMERA_DVP_D5_PIN                (18)
#define ESP_VISION_CAMERA_DVP_D6_PIN                (17)
#define ESP_VISION_CAMERA_DVP_D7_PIN                (16)

/* Display - SPI LCD 320x240 */
#define ESP_VISION_LCD_WIDTH                        (320)
#define ESP_VISION_LCD_HEIGHT                       (240)
#define ESP_VISION_LCD_SPI_HOST_ID                  (SPI2_HOST)
#define ESP_VISION_LCD_PIN_CS                       (2)
#define ESP_VISION_LCD_PIN_DC                       (1)
#define ESP_VISION_LCD_PIN_CLK                      (21)
#define ESP_VISION_LCD_PIN_MOSI                     (47)
#define ESP_VISION_LCD_PIN_RST                      (-1)
#define ESP_VISION_LCD_PIN_BL                       (14)
#define ESP_VISION_LCD_SPI_CLOCK_HZ                 (80000000)
#define ESP_VISION_LCD_BACKLIGHT_OUTPUT_INVERT      (0)

/* Touch - FT5x06 on I2C1 */
#define ESP_VISION_TOUCH_I2C_PORT                   (1)
#define ESP_VISION_TOUCH_I2C_SCL_PIN                (48)
#define ESP_VISION_TOUCH_I2C_SDA_PIN                (38)

/* Audio - ES8311 on I2C0 */
#define ESP_VISION_AUDIO_I2S_MCLK_PIN               (45)
#define ESP_VISION_AUDIO_I2S_WS_PIN                 (41)
#define ESP_VISION_AUDIO_I2S_BCLK_PIN               (39)
#define ESP_VISION_AUDIO_I2S_DIN_PIN                (40)
#define ESP_VISION_AUDIO_I2S_DOUT_PIN               (42)
#define ESP_VISION_AUDIO_PA_PIN                     (46)

/* No SD card */
#define ESP_VISION_SDCARD_ENABLE                    (0)

#endif /* ESP_VISION_BOARD_CONFIG_H */