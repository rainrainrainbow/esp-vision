/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

void esp_vision_debug_write(const char *str, size_t len);
void esp_vision_debug_vprintf(const char *fmt, va_list ap);
void esp_vision_debug_printf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif
