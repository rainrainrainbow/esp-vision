#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>
#include <driver/ledc.h>

// ============================================================
// 摄像头配置 (DVP, OV2640)
// ============================================================
#define ESP_VISION_CAMERA_SENSOR_ID              0x2640  // OV2640

// 输入/输出分辨率
#define ESP_VISION_CAMERA_RAW_INPUT_WIDTH        320
#define ESP_VISION_CAMERA_RAW_INPUT_HEIGHT       240
#define ESP_VISION_CAMERA_ACTIVE_INPUT_WIDTH     320
#define ESP_VISION_CAMERA_ACTIVE_INPUT_HEIGHT    240
#define ESP_VISION_CAMERA_BUFFER_COUNT           2

// 输出分辨率
#define ESP_VISION_CAMERA_OUTPUT_QQVGA_WIDTH     160
#define ESP_VISION_CAMERA_OUTPUT_QQVGA_HEIGHT    120
#define ESP_VISION_CAMERA_OUTPUT_QVGA_WIDTH      320
#define ESP_VISION_CAMERA_OUTPUT_QVGA_HEIGHT     240

// SCCB (I2C) 配置 - 用于摄像头控制
#define ESP_VISION_CAMERA_SCCB_I2C_PORT          0
#define ESP_VISION_CAMERA_SCCB_I2C_SCL_PIN       GPIO_NUM_5
#define ESP_VISION_CAMERA_SCCB_I2C_SDA_PIN       GPIO_NUM_4
#define ESP_VISION_CAMERA_SCCB_I2C_FREQ          400000

#define ESP_VISION_CAMERA_SENSOR_RESET_PIN       GPIO_NUM_NC
#define ESP_VISION_CAMERA_SENSOR_PWDN_PIN        GPIO_NUM_NC

// XCLK 配置
#define ESP_VISION_CAMERA_XCLK_PIN               GPIO_NUM_15
#define ESP_VISION_CAMERA_XCLK_FREQ              8000000
#define ESP_VISION_CAMERA_XCLK_LEDC_TIMER        LEDC_TIMER_0
#define ESP_VISION_CAMERA_XCLK_LEDC_CHANNEL      LEDC_CHANNEL_0

// DVP 数据引脚
#define ESP_VISION_CAMERA_DVP_PCLK_PIN           GPIO_NUM_13
#define ESP_VISION_CAMERA_DVP_VSYNC_PIN          GPIO_NUM_6
#define ESP_VISION_CAMERA_DVP_HSYNC_PIN          GPIO_NUM_7
#define ESP_VISION_CAMERA_DVP_D0_PIN             GPIO_NUM_11
#define ESP_VISION_CAMERA_DVP_D1_PIN             GPIO_NUM_9
#define ESP_VISION_CAMERA_DVP_D2_PIN             GPIO_NUM_8
#define ESP_VISION_CAMERA_DVP_D3_PIN             GPIO_NUM_10
#define ESP_VISION_CAMERA_DVP_D4_PIN             GPIO_NUM_12
#define ESP_VISION_CAMERA_DVP_D5_PIN             GPIO_NUM_18
#define ESP_VISION_CAMERA_DVP_D6_PIN             GPIO_NUM_17
#define ESP_VISION_CAMERA_DVP_D7_PIN             GPIO_NUM_16

// ============================================================
// LCD 显示配置 (SPI ST7789, 320x240)
// ============================================================
#define ESP_VISION_LCD_WIDTH                     320
#define ESP_VISION_LCD_HEIGHT                    240

#define ESP_VISION_LCD_SPI_HOST                  SPI3_HOST
#define ESP_VISION_LCD_PIXEL_CLOCK_HZ            (80 * 1000 * 1000)
#define ESP_VISION_LCD_BPP                       16

#define ESP_VISION_LCD_PIN_MOSI                  GPIO_NUM_47
#define ESP_VISION_LCD_PIN_CLK                   GPIO_NUM_21
#define ESP_VISION_LCD_PIN_CS                    GPIO_NUM_2
#define ESP_VISION_LCD_PIN_DC                    GPIO_NUM_1
#define ESP_VISION_LCD_PIN_RST                   GPIO_NUM_NC
#define ESP_VISION_LCD_PIN_BL                    GPIO_NUM_14

#define ESP_VISION_LCD_SPI_MODE                  0
#define ESP_VISION_LCD_TRANS_QUEUE_DEPTH         10

// 背光 PWM 配置
#define ESP_VISION_LCD_BACKLIGHT_CH              1
#define ESP_VISION_LCD_BACKLIGHT_TIMER           0
#define ESP_VISION_LCD_BACKLIGHT_PWM_HZ          5000
#define ESP_VISION_LCD_BACKLIGHT_OUTPUT_INVERT   0

// ============================================================
// SD 卡配置 - 不使用
// ============================================================

#endif // _BOARD_CONFIG_H_

// JPEG codec quality settings
#define ESP_VISION_IMLIB_JPEG_CODEC_ENABLE          (0)
#define ESP_VISION_JPEG_QUALITY_LOW                 (60)
#define ESP_VISION_JPEG_QUALITY_HIGH                (60)
#define ESP_VISION_JPEG_QUALITY_THRESHOLD           (320 * 240 * 2)
