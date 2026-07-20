/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "camera.h"

#include <inttypes.h>
#include <string.h>

#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"

#include "boardconfig.h"
#include "debug.h"
#include "jpeg.h"

#define pixformat_t         esp32_camera_pixformat_t
#define PIXFORMAT_RGB565    ESP32_CAMERA_PIXFORMAT_RGB565
#define PIXFORMAT_YUV422    ESP32_CAMERA_PIXFORMAT_YUV422
#define PIXFORMAT_YUV420    ESP32_CAMERA_PIXFORMAT_YUV420
#define PIXFORMAT_GRAYSCALE ESP32_CAMERA_PIXFORMAT_GRAYSCALE
#define PIXFORMAT_JPEG      ESP32_CAMERA_PIXFORMAT_JPEG
#define PIXFORMAT_RGB888    ESP32_CAMERA_PIXFORMAT_RGB888
#define PIXFORMAT_RAW       ESP32_CAMERA_PIXFORMAT_RAW
#define PIXFORMAT_RGB444    ESP32_CAMERA_PIXFORMAT_RGB444
#define PIXFORMAT_RGB555    ESP32_CAMERA_PIXFORMAT_RGB555
#define PIXFORMAT_RAW8      ESP32_CAMERA_PIXFORMAT_RAW8
#include "esp_camera.h"
#undef pixformat_t
#undef PIXFORMAT_RGB565
#undef PIXFORMAT_YUV422
#undef PIXFORMAT_YUV420
#undef PIXFORMAT_GRAYSCALE
#undef PIXFORMAT_JPEG
#undef PIXFORMAT_RGB888
#undef PIXFORMAT_RAW
#undef PIXFORMAT_RGB444
#undef PIXFORMAT_RGB555
#undef PIXFORMAT_RAW8

#ifndef CMSIS_MCU_H
#define CMSIS_MCU_H "cmsis_compiler.h"
#endif

#ifndef NO_QSTR
#include "imlib.h"
#endif

#define ESP_VISION_CAMERA_CAPTURE_RETRY_COUNT 3
#define ESP_VISION_CAMERA_JPEG_QUALITY        12

typedef struct {
    bool initialized;
    bool hmirror;
    bool vflip;
    uint32_t raw_input_width;
    uint32_t raw_input_height;
    uint32_t active_input_width;
    uint32_t active_input_height;
    uint32_t active_input_offset_x;
    uint32_t active_input_offset_y;
    uint32_t width;
    uint32_t height;
    pixformat_t output_pixfmt;
} esp_vision_camera_context_t;

static const char *TAG = "esp_vision_camera";
static esp_vision_camera_context_t s_camera = {
    .width = ESP_VISION_CAMERA_OUTPUT_QVGA_WIDTH,
    .height = ESP_VISION_CAMERA_OUTPUT_QVGA_HEIGHT,
    .output_pixfmt = PIXFORMAT_RGB565,
};

static void esp_vision_camera_set_dimensions(uint32_t width, uint32_t height)
{
    s_camera.raw_input_width = width;
    s_camera.raw_input_height = height;
    s_camera.active_input_width = width;
    s_camera.active_input_height = height;
    s_camera.active_input_offset_x = 0;
    s_camera.active_input_offset_y = 0;
    s_camera.width = width;
    s_camera.height = height;
}

static void esp_vision_camera_set_defaults(void)
{
    esp_vision_camera_set_dimensions(320, 240);
    s_camera.output_pixfmt = PIXFORMAT_RGB565;
}

void esp_vision_camera_init0(void)
{
    esp_vision_camera_deinit();
    memset(&s_camera, 0, sizeof(s_camera));
    esp_vision_camera_set_defaults();
}

static size_t esp_vision_camera_bpp(uint32_t pixfmt)
{
    switch (pixfmt) {
    case PIXFORMAT_GRAYSCALE:
        return sizeof(uint8_t);
    case PIXFORMAT_RGB565:
        return sizeof(uint16_t);
    default:
        return 0;
    }
}

static size_t esp_vision_camera_output_size(uint32_t width, uint32_t height, uint32_t pixfmt)
{
    return (size_t)width * (size_t)height * esp_vision_camera_bpp(pixfmt);
}

static esp_err_t esp_vision_camera_to_esp32_framesize(uint32_t width,
                                                      uint32_t height,
                                                      framesize_t *framesize)
{
    if (framesize == NULL) return ESP_ERR_INVALID_ARG;
    if ((width == 160) && (height == 120)) { *framesize = FRAMESIZE_QQVGA; return ESP_OK; }
    if ((width == 320) && (height == 240)) { *framesize = FRAMESIZE_QVGA; return ESP_OK; }
    if ((width == 640) && (height == 480)) { *framesize = FRAMESIZE_VGA; return ESP_OK; }
    return ESP_ERR_NOT_SUPPORTED;
}

static void esp_vision_camera_log_jpeg_frame(const char *reason, const camera_fb_t *fb)
{
    if ((fb == NULL) || (fb->buf == NULL) || (fb->len == 0)) {
        esp_vision_debug_printf("[esp-vision] camera jpeg %s: no frame\r\n", reason);
        return;
    }
    esp_vision_debug_printf("[esp-vision] camera jpeg %s: bytes=%u frame=%ux%u\r\n",
                            reason, (unsigned int)fb->len,
                            (unsigned int)fb->width, (unsigned int)fb->height);
}

static bool esp_vision_camera_get_jpeg_payload(const camera_fb_t *fb, image_t *src)
{
    if ((fb == NULL) || (src == NULL) || (fb->buf == NULL) || (fb->len < 4))
        return false;
    size_t start = fb->len, end = fb->len;
    for (size_t i = 0; (i + 1) < fb->len; i++) {
        if ((fb->buf[i] == 0xff) && (fb->buf[i + 1] == 0xd8)) { start = i; break; }
    }
    if (start == fb->len) { esp_vision_camera_log_jpeg_frame("missing soi", fb); return false; }
    for (size_t i = start + 2; (i + 1) < fb->len; i++) {
        if ((fb->buf[i] == 0xff) && (fb->buf[i + 1] == 0xd9)) { end = i + 2; break; }
    }
    *src = (image_t){ .w = fb->width, .h = fb->height, .pixfmt = PIXFORMAT_JPEG, .size = end - start, .pixels = fb->buf + start };
    return true;
}

esp_err_t esp_vision_camera_get_framesize_dimensions(esp_vision_camera_framesize_t framesize,
                                                     uint32_t *width, uint32_t *height)
{
    if (!width || !height) return ESP_ERR_INVALID_ARG;
    switch (framesize) {
    case ESP_VISION_CAMERA_FRAMESIZE_QQVGA: *width = 160; *height = 120; return ESP_OK;
    case ESP_VISION_CAMERA_FRAMESIZE_VGA:  *width = 640; *height = 480; return ESP_OK;
    case ESP_VISION_CAMERA_FRAMESIZE_QVGA:  *width = 320; *height = 240; return ESP_OK;
    default: return ESP_ERR_NOT_SUPPORTED;
    }
}

esp_err_t esp_vision_camera_init(void)
{
    if (s_camera.initialized) return ESP_OK;
    if ((s_camera.width == 0) || (s_camera.height == 0) || (s_camera.output_pixfmt == PIXFORMAT_INVALID))
        esp_vision_camera_set_defaults();

    framesize_t frame_size = FRAMESIZE_QVGA;
    esp_err_t ret = esp_vision_camera_to_esp32_framesize(s_camera.width, s_camera.height, &frame_size);
    if (ret != ESP_OK) return ret;

    const camera_config_t config = {
        .pin_pwdn = ESP_VISION_CAMERA_SENSOR_PWDN_PIN,
        .pin_reset = ESP_VISION_CAMERA_SENSOR_RESET_PIN,
        .pin_xclk = ESP_VISION_CAMERA_XCLK_PIN,
        .pin_sccb_sda = ESP_VISION_CAMERA_SCCB_I2C_SDA_PIN,
        .pin_sccb_scl = ESP_VISION_CAMERA_SCCB_I2C_SCL_PIN,
        .pin_d7 = ESP_VISION_CAMERA_DVP_D7_PIN,
        .pin_d6 = ESP_VISION_CAMERA_DVP_D6_PIN,
        .pin_d5 = ESP_VISION_CAMERA_DVP_D5_PIN,
        .pin_d4 = ESP_VISION_CAMERA_DVP_D4_PIN,
        .pin_d3 = ESP_VISION_CAMERA_DVP_D3_PIN,
        .pin_d2 = ESP_VISION_CAMERA_DVP_D2_PIN,
        .pin_d1 = ESP_VISION_CAMERA_DVP_D1_PIN,
        .pin_d0 = ESP_VISION_CAMERA_DVP_D0_PIN,
        .pin_vsync = ESP_VISION_CAMERA_DVP_VSYNC_PIN,
        .pin_href = ESP_VISION_CAMERA_DVP_HSYNC_PIN,
        .pin_pclk = ESP_VISION_CAMERA_DVP_PCLK_PIN,
        .xclk_freq_hz = ESP_VISION_CAMERA_XCLK_FREQ,
        .ledc_timer = (ledc_timer_t)ESP_VISION_CAMERA_XCLK_LEDC_TIMER,
        .ledc_channel = (ledc_channel_t)ESP_VISION_CAMERA_XCLK_LEDC_CHANNEL,
        .pixel_format = ESP32_CAMERA_PIXFORMAT_RGB565,
        .frame_size = frame_size,
        .jpeg_quality = 0,  // Not used for RGB565
        .fb_count = ESP_VISION_CAMERA_BUFFER_COUNT,
        .fb_location = CAMERA_FB_IN_PSRAM,
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
        .sccb_i2c_port = ESP_VISION_CAMERA_SCCB_I2C_PORT,
    };

    ret = esp_camera_init(&config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "failed to initialize esp32-camera: %s", esp_err_to_name(ret));
        return ret;
    }

    s_camera.vflip = true;
    s_camera.hmirror = true;
    s_camera.initialized = true;

    sensor_t *sensor = esp_camera_sensor_get();
    if (sensor != NULL) {
        sensor->set_hmirror(sensor, 1);
        sensor->set_vflip(sensor, 1);
    }
    return ESP_OK;
}

void esp_vision_camera_deinit(void)
{
    if (s_camera.initialized) {
        esp_camera_deinit();
        s_camera.initialized = false;
    }
}

bool esp_vision_camera_is_ready(void)
{
    return s_camera.initialized && s_camera.width != 0 && s_camera.height != 0;
}

esp_err_t esp_vision_camera_set_pixformat(uint32_t pixfmt)
{
    if ((pixfmt != PIXFORMAT_RGB565) && (pixfmt != PIXFORMAT_GRAYSCALE))
        return ESP_ERR_NOT_SUPPORTED;
    s_camera.output_pixfmt = (pixformat_t)pixfmt;
    return ESP_OK;
}

uint32_t esp_vision_camera_get_pixformat(void) { return s_camera.output_pixfmt; }

esp_err_t esp_vision_camera_set_framesize(esp_vision_camera_framesize_t framesize)
{
    uint32_t width = 0, height = 0;
    esp_err_t ret = esp_vision_camera_get_framesize_dimensions(framesize, &width, &height);
    if (ret != ESP_OK) return ret;
    if (s_camera.initialized) {
        framesize_t esp32_framesize = FRAMESIZE_QVGA;
        ret = esp_vision_camera_to_esp32_framesize(width, height, &esp32_framesize);
        if (ret != ESP_OK) return ret;
        sensor_t *sensor = esp_camera_sensor_get();
        if (!sensor || !sensor->set_framesize) return ESP_FAIL;
        if (sensor->set_framesize(sensor, esp32_framesize) != 0) return ESP_FAIL;
    }
    esp_vision_camera_set_dimensions(width, height);
    return ESP_OK;
}

uint32_t esp_vision_camera_get_width(void) { return s_camera.width; }
uint32_t esp_vision_camera_get_height(void) { return s_camera.height; }

esp_err_t esp_vision_camera_set_hmirror(bool enable)
{
    s_camera.hmirror = enable;
    sensor_t *sensor = esp_camera_sensor_get();
    if (sensor) sensor->set_hmirror(sensor, enable ? 1 : 0);
    return ESP_OK;
}
bool esp_vision_camera_get_hmirror(void) { return s_camera.hmirror; }

esp_err_t esp_vision_camera_set_vflip(bool enable)
{
    s_camera.vflip = enable;
    sensor_t *sensor = esp_camera_sensor_get();
    if (sensor) sensor->set_vflip(sensor, enable ? 1 : 0);
    return ESP_OK;
}
bool esp_vision_camera_get_vflip(void) { return s_camera.vflip; }
uint32_t esp_vision_camera_get_sensor_id(void) { return ESP_VISION_CAMERA_SENSOR_ID; }
size_t esp_vision_camera_frame_size(void)
{
    return esp_vision_camera_output_size(s_camera.width, s_camera.height, s_camera.output_pixfmt);
}

esp_err_t esp_vision_camera_capture(uint8_t *pixels, size_t pixels_size)
{
    if (!pixels || !esp_vision_camera_is_ready()) return ESP_ERR_INVALID_STATE;
    size_t expected_size = esp_vision_camera_frame_size();
    if ((expected_size == 0) || (pixels_size < expected_size)) return ESP_ERR_INVALID_SIZE;

    for (int attempt = 0; attempt < 3; attempt++) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) return ESP_FAIL;
        esp_err_t ret = ESP_ERR_INVALID_RESPONSE;
        size_t fb_size = fb->len;
        size_t expected = esp_vision_camera_output_size(s_camera.width, s_camera.height, s_camera.output_pixfmt);
        size_t total_pixels = (size_t)s_camera.width * s_camera.height;
        size_t total_bytes = total_pixels * 2;
        if (fb->format == ESP32_CAMERA_PIXFORMAT_RGB565 && fb_size >= total_bytes) {
            uint8_t *src = fb->buf;
            uint8_t *dst = pixels;
            if (s_camera.output_pixfmt == PIXFORMAT_GRAYSCALE) {
                // Byte swap first (GC2145 high byte first -> little-endian), then RGB565 -> Y
                for (size_t j = 0; j < total_bytes; j += 2) {
                    uint8_t hi = src[j];    // high byte from sensor (big-endian: first byte)
                    uint8_t lo = src[j+1];  // low byte from sensor (big-endian: second byte)
                    uint16_t pixel = (uint16_t)(hi << 8) | lo;  // big-endian -> 16-bit
                    // Extract RGB565 components (big-endian format)
                    uint8_t r5 = (hi >> 3) & 0x1F;
                    uint8_t g6 = ((hi & 0x07) << 3) | (lo >> 5);
                    uint8_t b5 = lo & 0x1F;
                    // Scale to 8-bit
                    uint8_t r8 = (r5 << 3) | (r5 >> 2);
                    uint8_t g8 = (g6 << 2) | (g6 >> 4);
                    uint8_t b8 = (b5 << 3) | (b5 >> 2);
                    // Y = 0.299R + 0.587G + 0.114B  (integer approx)
                    dst[j/2] = (uint8_t)((77 * r8 + 150 * g8 + 29 * b8) >> 8);
                }
                ret = ESP_OK;
            } else {
                // RGB565: byte swap (GC2145 high byte first -> little-endian)
                for (size_t j = 0; j < total_bytes; j += 2) {
                    dst[j] = src[j+1];
                    dst[j+1] = src[j];
                }
                ret = ESP_OK;
            }
        }
        esp_camera_fb_return(fb);
        if (ret != ESP_ERR_INVALID_RESPONSE) return ret;
    }
    return ESP_ERR_INVALID_RESPONSE;
}

esp_err_t esp_vision_camera_snapshot(image_t *img, uint8_t *pixels, size_t pixels_size)
{
    if (!img) return ESP_ERR_INVALID_ARG;
    esp_err_t ret = esp_vision_camera_capture(pixels, pixels_size);
    if (ret != ESP_OK) return ret;
    img->w = s_camera.width; img->h = s_camera.height;
    img->pixfmt = s_camera.output_pixfmt; img->size = 0;
    img->_raw = NULL; img->pixels = pixels;
    return ESP_OK;
}

esp_err_t esp_vision_camera_get_status(esp_vision_camera_status_t *status)
{
    if (!status) return ESP_ERR_INVALID_ARG;
    status->ready = esp_vision_camera_is_ready();
    status->sensor_id = ESP_VISION_CAMERA_SENSOR_ID;
    status->raw_input_width = s_camera.raw_input_width;
    status->raw_input_height = s_camera.raw_input_height;
    status->active_input_width = s_camera.active_input_width;
    status->active_input_height = s_camera.active_input_height;
    status->active_input_offset_x = s_camera.active_input_offset_x;
    status->active_input_offset_y = s_camera.active_input_offset_y;
    status->width = s_camera.width;
    status->height = s_camera.height;
    status->pixfmt = s_camera.output_pixfmt;
    status->hmirror = s_camera.hmirror;
    status->vflip = s_camera.vflip;
    return ESP_OK;
}

