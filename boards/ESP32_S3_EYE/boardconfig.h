#ifndef __BOARDCONFIG_H__
#define __BOARDCONFIG_H__

// Board: ESP32-S3-EYE

#define BOARD_HAS_PSRAM
#define BOARD_HAS_PDM
#ifndef BOARD_MAX_FRAMEBUFFER_RES
#  define BOARD_MAX_FRAMEBUFFER_RES VGA    // 640x480
#endif
#ifndef BOARD_HAS_SPIFLASH
#  define BOARD_HAS_SPIFLASH
#endif
#ifndef CONFIG_BOARD_SDCARD_PIN_CS_DEFAULT
#  define CONFIG_BOARD_SDCARD_PIN_CS_DEFAULT -1
#endif

// Camera sensor
#define BOARD_SENSOR_ID             0x2145  // GC2145
#define BOARD_SENSOR_PID            GC2145_PID
#define BOARD_SENSOR_WIDTH          640
#define BOARD_SENSOR_HEIGHT         480
#define BOARD_SENSOR_FPS            30
#define BOARD_SENSOR_FORMAT         0
#define BOARD_SENSOR_USE_AF         0
#define BOARD_CAMERA_XCLK_FREQ      20000000
#define BOARD_CAMERA_HREF           0
#define BOARD_CAMERA_VSYNC          0
#define BOARD_CAMERA_PCLK           0
#define BOARD_CAMERA_D0             15
#define BOARD_CAMERA_D1             17
#define BOARD_CAMERA_D2             16
#define BOARD_CAMERA_D3             18
#define BOARD_CAMERA_D4             14
#define BOARD_CAMERA_D5             13
#define BOARD_CAMERA_D6             12
#define BOARD_CAMERA_D7             11
#define BOARD_CAMERA_XCLK           10
#define BOARD_CAMERA_RESET          48
#define BOARD_CAMERA_PWDN           -1
#define BOARD_CAMERA_SIOD           4
#define BOARD_CAMERA_SIOC           5

// SCCB tweak
#define BOARD_SENSOR_SCCB_INTERNAL_PULLUP 0

// XCLK stable delay
#define BOARD_SENSOR_XCLK_STABLE_AFTER_POWER_UP_MS 100

// Video device settings
#define BOARD_VIDEO_DEV_DVP_ENABLE               1

// LCD
#define BOARD_LCD_BK_LIGHT_GPIO          48
#define BOARD_LCD_SPI_HOST               SPI2_HOST
#define BOARD_LCD_SPI_MOSI               40
#define BOARD_LCD_SPI_SCLK               41
#define BOARD_LCD_SPI_CS                 39
#define BOARD_LCD_SPI_DC                 42
#define BOARD_LCD_SPI_RST                -1

// LCD display params
#define BOARD_LCD_H_RES                  240
#define BOARD_LCD_V_RES                  240
#define BOARD_LCD_CMD_BITS               8
#define BOARD_LCD_PARAM_BITS             8

// LCD (ST7789)
#define BOARD_LCD_PIXEL_CLOCK_HZ         (40 * 1000 * 1000)
#define BOARD_LCD_BK_LIGHT_ON_LEVEL      1
#define BOARD_LCD_BK_LIGHT_OFF_LEVEL     !BOARD_LCD_BK_LIGHT_ON_LEVEL
#define BOARD_LCD_INVERT_COLOR           0
#define BOARD_LCD_SWAP_XY                0
#define BOARD_LCD_MIRROR_X               1
#define BOARD_LCD_MIRROR_Y               0

// Audio
#define BOARD_PDM_MIC_SEL_GPIO    -1
#define BOARD_PDM_I2S_CLK_GPIO    2
#define BOARD_PDM_I2S_DIN_GPIO    1

// SD card (SPI mode)
#define BOARD_SDCARD_SPI_HOST     SPI3_HOST
#define BOARD_SDCARD_SPI_SCK      7
#define BOARD_SDCARD_SPI_MOSI     8
#define BOARD_SDCARD_SPI_MISO     9
#define BOARD_SDCARD_SPI_CS       21
#define BOARD_SDCARD_PIN_CD       -1
#define BOARD_SDCARD_PIN_WP       -1

#endif // __BOARDCONFIG_H__
