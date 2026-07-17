/*
 * ESP32-S3-EYE Board Configuration
 * ESP-VISION framework, GC2145 DVP camera
 */

#ifndef ESP_VISION_BOARD_CONFIG_H
#define ESP_VISION_BOARD_CONFIG_H

#define ESP_VISION_BOARD_ARCH                  "ESP32S3"
#define ESP_VISION_BOARD_TYPE                  "ESP32_S3_EYE"
#define ESP_VISION_PORT_ESP32                       (1)

#define ESP_VISION_IMLIB_PROFILER_ENABLE            (0)
#define ESP_VISION_IMLIB_GPU_ENABLE                 (0)
#define ESP_VISION_IMLIB_JPEG_CODEC_ENABLE          (0)

#define ESP_VISION_CACHE_LINE_SIZE                  (32)
#define ESP_VISION_ALLOC_ALIGNMENT                  (ESP_VISION_CACHE_LINE_SIZE)
#define ESP_VISION_DMA_ALIGNMENT                    (ESP_VISION_CACHE_LINE_SIZE)

/* Camera configuration — GC2145 */
#define ESP_VISION_CAMERA_SENSOR_ID                 (0x2145)
#define ESP_VISION_CAMERA_RAW_INPUT_WIDTH           (640)
#define ESP_VISION_CAMERA_RAW_INPUT_HEIGHT          (480)
#define ESP_VISION_CAMERA_ACTIVE_INPUT_WIDTH        (640)
#define ESP_VISION_CAMERA_ACTIVE_INPUT_HEIGHT       (480)
#define ESP_VISION_CAMERA_OUTPUT_QQVGA_WIDTH        (160)
#define ESP_VISION_CAMERA_OUTPUT_QQVGA_HEIGHT       (120)
#define ESP_VISION_CAMERA_OUTPUT_QVGA_WIDTH         (320)
#define ESP_VISION_CAMERA_OUTPUT_QVGA_HEIGHT        (240)
#define ESP_VISION_CAMERA_BUFFER_COUNT              (3)
#define ESP_VISION_CAMERA_SCCB_I2C_PORT             (1)
#define ESP_VISION_CAMERA_SCCB_I2C_SCL_PIN          (5)
#define ESP_VISION_CAMERA_SCCB_I2C_SDA_PIN          (4)
#define ESP_VISION_CAMERA_SENSOR_RESET_PIN          (48)
#define ESP_VISION_CAMERA_SENSOR_PWDN_PIN           (-1)
#define ESP_VISION_CAMERA_SCCB_I2C_FREQ             (100000)
#define ESP_VISION_CAMERA_SCCB_INTERNAL_PULLUP      (0)
#define ESP_VISION_CAMERA_XCLK_PIN                  (10)
#define ESP_VISION_CAMERA_XCLK_FREQ                 (20000000)
#define ESP_VISION_CAMERA_XCLK_STABLE_MS            (100)
#define ESP_VISION_CAMERA_XCLK_LEDC_TIMER           (1)
#define ESP_VISION_CAMERA_XCLK_LEDC_CHANNEL         (0)
#define ESP_VISION_CAMERA_DVP_PCLK_PIN              (8)
#define ESP_VISION_CAMERA_DVP_VSYNC_PIN             (6)
#define ESP_VISION_CAMERA_DVP_HSYNC_PIN             (7)
#define ESP_VISION_CAMERA_DVP_D0_PIN                (15)
#define ESP_VISION_CAMERA_DVP_D1_PIN                (17)
#define ESP_VISION_CAMERA_DVP_D2_PIN                (16)
#define ESP_VISION_CAMERA_DVP_D3_PIN                (18)
#define ESP_VISION_CAMERA_DVP_D4_PIN                (14)
#define ESP_VISION_CAMERA_DVP_D5_PIN                (13)
#define ESP_VISION_CAMERA_DVP_D6_PIN                (12)
#define ESP_VISION_CAMERA_DVP_D7_PIN                (11)

/* LCD Display */
#define ESP_VISION_LCD_WIDTH                        (240)
#define ESP_VISION_LCD_HEIGHT                       (240)
#define ESP_VISION_LCD_BPP                          (16)
#define ESP_VISION_LCD_PIXEL_CLOCK_HZ               (40 * 1000 * 1000)

/* SD Card */
#define ESP_VISION_SDCARD_MOUNT_PATH                "/sdcard"
#define ESP_VISION_SDCARD_SLOT                      (0)
#define ESP_VISION_SDCARD_BUS_WIDTH                 (1)

#endif /* ESP_VISION_BOARD_CONFIG_H */
