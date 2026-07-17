/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * MicroPython audio module for HIWONDER_AI_MODULE
 * - ES8311 I2S audio output via board audio driver
 * - MP3 decoding via minimp3
 * - PCM playback
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"

#include "boardconfig.h"

/* Board-level audio functions (from boards/HIWONDER_AI_MODULE/audio.c) */
esp_err_t esp_vision_audio_init(void);
void esp_vision_audio_deinit(void);
bool esp_vision_audio_is_ready(void);
int esp_vision_audio_play_pcm(const uint8_t *data, size_t len, uint32_t sample_rate);

/* minimp3 header */
#include "minimp3.h"
#include "minimp3_ex.h"

static const char *TAG = "py_audio";

/*------------------------------------------------------------------*/
/* mp3_to_pcm: decode MP3 to raw PCM using minimp3                  */
/* Returns allocated PCM buffer (caller must free) and fills size   */
/*------------------------------------------------------------------*/
static uint8_t *mp3_to_pcm(const uint8_t *mp3_data, size_t mp3_len,
                           size_t *out_len, int *sample_rate)
{
    mp3dec_t mp3d;
    mp3dec_file_info_t info;
    mp3dec_init(&mp3d);

    int ret = mp3dec_load_buf(&mp3d, mp3_data, mp3_len, &info, NULL, NULL);
    if (ret != 0) {
        return NULL;
    }

    *out_len = info.samples * sizeof(int16_t);
    *sample_rate = info.hz;
    return (uint8_t *)info.buffer;
}

/*------------------------------------------------------------------*/
/* MicroPython: audio.init()                                        */
/*------------------------------------------------------------------*/
static mp_obj_t py_audio_init(void)
{
    esp_err_t ret = esp_vision_audio_init();
    if (ret != ESP_OK) {
        mp_raise_msg(&mp_type_RuntimeError, "audio init failed");
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(py_audio_init_obj, py_audio_init);

/*------------------------------------------------------------------*/
/* MicroPython: audio.deinit()                                      */
/*------------------------------------------------------------------*/
static mp_obj_t py_audio_deinit(void)
{
    esp_vision_audio_deinit();
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(py_audio_deinit_obj, py_audio_deinit);

/*------------------------------------------------------------------*/
/* MicroPython: audio.is_ready()                                    */
/*------------------------------------------------------------------*/
static mp_obj_t py_audio_is_ready(void)
{
    return mp_obj_new_bool(esp_vision_audio_is_ready());
}
static MP_DEFINE_CONST_FUN_OBJ_0(py_audio_is_ready_obj, py_audio_is_ready);

/*------------------------------------------------------------------*/
/* MicroPython: audio.play_pcm(data, sample_rate)                   */
/*------------------------------------------------------------------*/
static mp_obj_t py_audio_play_pcm(mp_obj_t data_obj, mp_obj_t rate_obj)
{
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(data_obj, &bufinfo, MP_BUFFER_READ);

    mp_int_t sample_rate = mp_obj_get_int(rate_obj);
    if (sample_rate <= 0) {
        mp_raise_ValueError("sample_rate must be > 0");
    }

    int ret = esp_vision_audio_play_pcm((const uint8_t *)bufinfo.buf,
                                         bufinfo.len,
                                         (uint32_t)sample_rate);
    if (ret < 0) {
        mp_raise_msg(&mp_type_RuntimeError, "play_pcm failed");
    }
    return mp_obj_new_int(ret);
}
static MP_DEFINE_CONST_FUN_OBJ_2(py_audio_play_pcm_obj, py_audio_play_pcm);

/*------------------------------------------------------------------*/
/* MicroPython: audio.play_mp3(mp3_data)                            */
/*------------------------------------------------------------------*/
static mp_obj_t py_audio_play_mp3(mp_obj_t data_obj)
{
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(data_obj, &bufinfo, MP_BUFFER_READ);

    size_t pcm_len = 0;
    int sample_rate = 0;
    uint8_t *pcm = mp3_to_pcm((const uint8_t *)bufinfo.buf, bufinfo.len,
                               &pcm_len, &sample_rate);
    if (!pcm) {
        mp_raise_msg(&mp_type_RuntimeError, "MP3 decode failed");
    }

    int written = esp_vision_audio_play_pcm(pcm, pcm_len, (uint32_t)sample_rate);
    free(pcm);

    if (written < 0) {
        mp_raise_msg(&mp_type_RuntimeError, "play_mp3 failed");
    }
    return mp_obj_new_int(written);
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_audio_play_mp3_obj, py_audio_play_mp3);

/*------------------------------------------------------------------*/
/* Module definition                                                */
/*------------------------------------------------------------------*/
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