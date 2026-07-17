// Audio driver for HIWONDER_AI_MODULE (ES8311 + I2S)
// Provides audio init, deinit, and PCM playback

#ifndef ESP_VISION_AUDIO_H__
#define ESP_VISION_AUDIO_H__

#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

// Functions defined in audio.c
esp_err_t esp_vision_audio_init(void);
void esp_vision_audio_deinit(void);
bool esp_vision_audio_is_ready(void);
int esp_vision_audio_play_pcm(const uint8_t *data, size_t len, uint32_t sample_rate);

#endif /* ESP_VISION_AUDIO_H__ */
