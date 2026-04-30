/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "debug.h"

#include <stdio.h>

#include "py/mpconfig.h"
#include "usb_serial_jtag.h"

void esp_vision_debug_write(const char *str, size_t len)
{
    if ((str == NULL) || (len == 0)) {
        return;
    }

#if MICROPY_HW_ESP_USB_SERIAL_JTAG
    usb_serial_jtag_tx_strn(str, len);
#else
    fwrite(str, 1, len, stdout);
#endif
}

void esp_vision_debug_vprintf(const char *fmt, va_list ap)
{
    char buf[256];
    int len = vsnprintf(buf, sizeof(buf), fmt, ap);

    if (len <= 0) {
        return;
    }

    size_t write_len = (size_t)len;
    if (write_len >= sizeof(buf)) {
        write_len = sizeof(buf) - 1;
    }

    esp_vision_debug_write(buf, write_len);
}

void esp_vision_debug_printf(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    esp_vision_debug_vprintf(fmt, ap);
    va_end(ap);
}
