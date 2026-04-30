/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "preview.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "driver/jpeg_encode.h"
#include "esp_check.h"
#include "mbedtls/base64.h"
#include "py/mpconfig.h"
#include "py/mphal.h"

#if MICROPY_HW_USB_CDC
#include "shared/tinyusb/mp_usbd_cdc.h"
#endif

#ifndef CMSIS_MCU_H
#define CMSIS_MCU_H "cmsis_compiler.h"
#endif

#ifndef NO_QSTR
#include "imlib.h"
#endif

#include "boardconfig.h"

#define ESP_VISION_PREVIEW_BASE64_INPUT_CHUNK (768)
#define ESP_VISION_PREVIEW_BASE64_OUTPUT_CHUNK ((ESP_VISION_PREVIEW_BASE64_INPUT_CHUNK / 3) * 4)
#define ESP_VISION_PREVIEW_JPEG_TIMEOUT_MS (200)

typedef struct {
    jpeg_encoder_handle_t encoder;
    uint8_t *input_buf;
    size_t input_buf_size;
    uint8_t *jpeg_buf;
    size_t jpeg_buf_size;
} esp_vision_preview_context_t;

static const char *TAG = "esp_vision_preview";
static esp_vision_preview_context_t s_preview;

static esp_err_t esp_vision_preview_write(const char *str, size_t len)
{
    if ((str == NULL) || (len == 0)) {
        return ESP_OK;
    }

#if MICROPY_HW_USB_CDC
    mp_uint_t written = mp_usbd_cdc_tx_strn(str, len);
#else
    mp_uint_t written = mp_hal_stdout_tx_strn(str, len);
#endif
    return (written == len) ? ESP_OK : ESP_FAIL;
}

static esp_err_t esp_vision_preview_printf(const char *fmt, ...)
{
    char buf[160];
    va_list ap;

    va_start(ap, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if (len <= 0) {
        return ESP_FAIL;
    }
    if ((size_t)len >= sizeof(buf)) {
        return ESP_ERR_INVALID_SIZE;
    }

    return esp_vision_preview_write(buf, (size_t)len);
}

static void esp_vision_preview_free_buffers(void)
{
    if (s_preview.input_buf != NULL) {
        free(s_preview.input_buf);
        s_preview.input_buf = NULL;
    }
    if (s_preview.jpeg_buf != NULL) {
        free(s_preview.jpeg_buf);
        s_preview.jpeg_buf = NULL;
    }
    s_preview.input_buf_size = 0;
    s_preview.jpeg_buf_size = 0;
}

void esp_vision_preview_deinit(void)
{
    if (s_preview.encoder != NULL) {
        jpeg_del_encoder_engine(s_preview.encoder);
        s_preview.encoder = NULL;
    }
    esp_vision_preview_free_buffers();
}

void esp_vision_preview_init0(void)
{
    esp_vision_preview_deinit();
}

static esp_err_t esp_vision_preview_ensure_encoder(void)
{
    if (s_preview.encoder != NULL) {
        return ESP_OK;
    }

    jpeg_encode_engine_cfg_t encoder_config = {
        .intr_priority = 0,
        .timeout_ms = ESP_VISION_PREVIEW_JPEG_TIMEOUT_MS,
    };

    return jpeg_new_encoder_engine(&encoder_config, &s_preview.encoder);
}

static size_t esp_vision_preview_jpeg_capacity(size_t raw_size)
{
    size_t capacity = raw_size + (raw_size / 2) + 4096;
    return (capacity < 16384) ? 16384 : capacity;
}

static esp_err_t esp_vision_preview_ensure_buffers(size_t raw_size)
{
    if (s_preview.input_buf_size < raw_size) {
        if (s_preview.input_buf != NULL) {
            free(s_preview.input_buf);
            s_preview.input_buf = NULL;
            s_preview.input_buf_size = 0;
        }

        jpeg_encode_memory_alloc_cfg_t input_config = {
            .buffer_direction = JPEG_ENC_ALLOC_INPUT_BUFFER,
        };
        s_preview.input_buf = jpeg_alloc_encoder_mem(raw_size, &input_config, &s_preview.input_buf_size);
        if (s_preview.input_buf == NULL) {
            return ESP_ERR_NO_MEM;
        }
    }

    size_t jpeg_capacity = esp_vision_preview_jpeg_capacity(raw_size);
    if (s_preview.jpeg_buf_size < jpeg_capacity) {
        if (s_preview.jpeg_buf != NULL) {
            free(s_preview.jpeg_buf);
            s_preview.jpeg_buf = NULL;
            s_preview.jpeg_buf_size = 0;
        }

        jpeg_encode_memory_alloc_cfg_t output_config = {
            .buffer_direction = JPEG_ENC_ALLOC_OUTPUT_BUFFER,
        };
        s_preview.jpeg_buf = jpeg_alloc_encoder_mem(jpeg_capacity, &output_config, &s_preview.jpeg_buf_size);
        if (s_preview.jpeg_buf == NULL) {
            return ESP_ERR_NO_MEM;
        }
    }

    return ESP_OK;
}

static esp_err_t esp_vision_preview_image_format(const image_t *img,
                                                 size_t *raw_size,
                                                 jpeg_enc_input_format_t *src_type,
                                                 jpeg_down_sampling_type_t *sub_sample)
{
    if ((img == NULL) || (img->pixels == NULL) || (img->w <= 0) || (img->h <= 0)) {
        return ESP_ERR_INVALID_ARG;
    }

    switch (img->pixfmt) {
    case PIXFORMAT_GRAYSCALE:
        *raw_size = (size_t)img->w * (size_t)img->h;
        *src_type = JPEG_ENCODE_IN_FORMAT_GRAY;
        *sub_sample = JPEG_DOWN_SAMPLING_GRAY;
        return ESP_OK;
    case PIXFORMAT_RGB565:
        *raw_size = (size_t)img->w * (size_t)img->h * sizeof(uint16_t);
        *src_type = JPEG_ENCODE_IN_FORMAT_RGB565;
        *sub_sample = JPEG_DOWN_SAMPLING_YUV422;
        return ESP_OK;
    default:
        return ESP_ERR_NOT_SUPPORTED;
    }
}

static esp_err_t esp_vision_preview_encode_jpeg(const image_t *img, uint8_t **jpeg_buf, size_t *jpeg_size)
{
    size_t raw_size = 0;
    jpeg_enc_input_format_t src_type = JPEG_ENCODE_IN_FORMAT_RGB565;
    jpeg_down_sampling_type_t sub_sample = JPEG_DOWN_SAMPLING_YUV422;

    ESP_RETURN_ON_ERROR(esp_vision_preview_image_format(img, &raw_size, &src_type, &sub_sample),
                        TAG,
                        "unsupported preview image");
    ESP_RETURN_ON_ERROR(esp_vision_preview_ensure_encoder(), TAG, "failed to create jpeg encoder");
    ESP_RETURN_ON_ERROR(esp_vision_preview_ensure_buffers(raw_size), TAG, "failed to allocate preview buffers");

    memcpy(s_preview.input_buf, img->pixels, raw_size);

    jpeg_encode_cfg_t encode_config = {
        .height = (uint32_t)img->h,
        .width = (uint32_t)img->w,
        .src_type = src_type,
        .sub_sample = sub_sample,
        .image_quality = ESP_VISION_JPEG_QUALITY_LOW,
    };

    uint32_t out_size = 0;
    esp_err_t ret = jpeg_encoder_process(s_preview.encoder,
                                         &encode_config,
                                         s_preview.input_buf,
                                         (uint32_t)raw_size,
                                         s_preview.jpeg_buf,
                                         (uint32_t)s_preview.jpeg_buf_size,
                                         &out_size);
    if (ret != ESP_OK) {
        return ret;
    }

    *jpeg_buf = s_preview.jpeg_buf;
    *jpeg_size = out_size;
    return ESP_OK;
}

static esp_err_t esp_vision_preview_write_base64(const uint8_t *data, size_t len)
{
    unsigned char out[ESP_VISION_PREVIEW_BASE64_OUTPUT_CHUNK + 1];
    size_t offset = 0;

    while (offset < len) {
        size_t chunk = len - offset;
        if (chunk > ESP_VISION_PREVIEW_BASE64_INPUT_CHUNK) {
            chunk = ESP_VISION_PREVIEW_BASE64_INPUT_CHUNK;
        }

        size_t out_len = 0;
        int ret = mbedtls_base64_encode(out, sizeof(out), &out_len, data + offset, chunk);
        if (ret != 0) {
            return ESP_FAIL;
        }

        ESP_RETURN_ON_ERROR(esp_vision_preview_write((const char *)out, out_len),
                            TAG,
                            "failed to write preview frame");
        offset += chunk;
    }

    return ESP_OK;
}

static esp_err_t esp_vision_preview_write_frame(const image_t *img, const uint8_t *jpeg_buf, size_t jpeg_size)
{
    ESP_RETURN_ON_ERROR(esp_vision_preview_printf("<EVFRAME width=%" PRIi32 " height=%" PRIi32
                                                  " format=jpg size=%u encoding=base64>\r\n",
                                                  img->w,
                                                  img->h,
                                                  (unsigned int)jpeg_size),
                        TAG,
                        "failed to write preview header");

    ESP_RETURN_ON_ERROR(esp_vision_preview_write_base64(jpeg_buf, jpeg_size), TAG, "failed to encode preview frame");
    return esp_vision_preview_write("\r\n</EVFRAME>\r\n", strlen("\r\n</EVFRAME>\r\n"));
}

esp_err_t esp_vision_preview_flush(const image_t *img)
{
    uint8_t *jpeg_buf = NULL;
    size_t jpeg_size = 0;

    ESP_RETURN_ON_ERROR(esp_vision_preview_encode_jpeg(img, &jpeg_buf, &jpeg_size), TAG, "failed to encode jpeg");
    return esp_vision_preview_write_frame(img, jpeg_buf, jpeg_size);
}
