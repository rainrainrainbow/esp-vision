/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * MicroPython audio module for HIWONDER_AI_MODULE
 * - ES8311 I2S audio output via board audio driver
 * - MP3 decoding via minimp3 (core API, no minimp3_ex.h needed)
 * - PCM playback
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"

#include "boardconfig.h"

/* Board-level audio functions */
esp_err_t esp_vision_audio_init(void);
void esp_vision_audio_deinit(void);
bool esp_vision_audio_is_ready(void);
int esp_vision_audio_play_pcm(const uint8_t *data, size_t len, uint32_t sample_rate);

#include "minimp3.h"

static const char *TAG = "py_audio";

static uint8_t *mp3_to_pcm(const uint8_t *mp3_data, size_t mp3_len,
                           size_t *out_len, int *sample_rate)
{
    mp3dec_t mp3d;
    mp3dec_init(&mp3d);

    int16_t dummy[MINIMP3_MAX_SAMPLES_PER_FRAME];
    mp3dec_frame_info_t info;
    size_t offset = 0;
    int detected_hz = 0;
    size_t total_samples = 0;

    while (offset < mp3_len) {
        int samples = mp3dec_decode_frame(&mp3d, mp3_data + offset,
                                          (int)(mp3_len - offset),
                                          dummy, &info);
        if (samples > 0) {
            if (detected_hz == 0) {
                detected_hz = info.hz;
            }
            total_samples += (size_t)samples;
        }
        if (info.frame_bytes == 0) break;
        offset += info.frame_bytes;
    }

    if (detected_hz == 0 || total_samples == 0) {
        return NULL;
    }

    mp3dec_init(&mp3d);
    size_t pcm_size = total_samples * sizeof(int16_t);
    int16_t *pcm_buf = (int16_t *)malloc(pcm_size);
    if (!pcm_buf) return NULL;

    offset = 0;
    size_t written = 0;
    while (offset < mp3_len && written < total_samples) {
        int samples = mp3dec_decode_frame(&mp3d, mp3_data + offset,
                                          (int)(mp3_len - offset),
                                          pcm_buf + written, &info);
        if (samples > 0) {
            written += (size_t)samples;
        }
        if (info.frame_bytes == 0) break;
        offset += info.frame_bytes;
    }

    *out_len = written * sizeof(int16_t);
    *sample_rate = detected_hz;
    return (uint8_t *)pcm_buf;
}

static mp_obj_t py_audio_init(void)
{
    esp_err_t ret = esp_vision_audio_init();
    if (ret != ESP_OK) {
        mp_raise_msg_varg(&mp_type_RuntimeError, "audio init failed: %s", esp_err_to_name(ret));
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(py_audio_init_obj, py_audio_init);

static mp_obj_t py_audio_deinit(void)
{
    esp_vision_audio_deinit();
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(py_audio_deinit_obj, py_audio_deinit);

static mp_obj_t py_audio_is_ready(void)
{
    return mp_obj_new_bool(esp_vision_audio_is_ready());
}
static MP_DEFINE_CONST_FUN_OBJ_0(py_audio_is_ready_obj, py_audio_is_ready);

static mp_obj_t py_audio_play_pcm(mp_obj_t data_obj, mp_obj_t rate_obj)
{
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(data_obj, &bufinfo, MP_BUFFER_READ);

    mp_int_t sample_rate = mp_obj_get_int(rate_obj);
    if (sample_rate <= 0) {
        mp_raise_ValueError(MP_ERROR_TEXT("sample_rate must be > 0"));
    }

    int ret = esp_vision_audio_play_pcm((const uint8_t *)bufinfo.buf,
                                         bufinfo.len,
                                         (uint32_t)sample_rate);
    if (ret < 0) {
        mp_raise_msg_varg(&mp_type_RuntimeError, "play_pcm failed: ret=%d", ret);
    }
    return mp_obj_new_int(ret);
}
static MP_DEFINE_CONST_FUN_OBJ_2(py_audio_play_pcm_obj, py_audio_play_pcm);

static mp_obj_t py_audio_play_mp3(mp_obj_t data_obj)
{
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(data_obj, &bufinfo, MP_BUFFER_READ);

    size_t pcm_len = 0;
    int sample_rate = 0;
    uint8_t *pcm = mp3_to_pcm((const uint8_t *)bufinfo.buf, bufinfo.len,
                               &pcm_len, &sample_rate);
    if (!pcm) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("MP3 decode failed"));
    }

    int written = esp_vision_audio_play_pcm(pcm, pcm_len, (uint32_t)sample_rate);
    free(pcm);

    if (written < 0) {
        mp_raise_msg_varg(&mp_type_RuntimeError, "play_mp3 failed: ret=%d", written);
    }
    return mp_obj_new_int(written);
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_audio_play_mp3_obj, py_audio_play_mp3);

static const mp_rom_map_elem_t audio_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),    MP_ROM_QSTR(MP_QSTR_audio) },
    { MP_ROM_QSTR(MP_QSTR_init),        MP_ROM_PTR(&py_audio_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit),      MP_ROM_PTR(&py_audio_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_is_ready),    MP_ROM_PTR(&py_audio_is_ready_obj) },
    { MP_ROM_QSTR(MP_QSTR_play_pcm),    MP_ROM_PTR(&py_audio_play_pcm_obj) },
    { MP_ROM_QSTR(MP_QSTR_play_mp3),    MP_ROM_PTR(&py_audio_play_mp3_obj) },
};
static MP_DEFINE_CONST_DICT(audio_module_globals, audio_module_globals_table);

const mp_obj_module_t audio_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&audio_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_audio, audio_user_cmodule);
