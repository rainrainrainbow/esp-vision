/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: MIT
 */

#ifndef ESP_VISION_IMLIB_CMSIS_EXTENSION_H
#define ESP_VISION_IMLIB_CMSIS_EXTENSION_H

#include "cmsis_compiler.h"

__STATIC_FORCEINLINE int32_t __SSAT_ASR(int32_t value, uint32_t sat, uint32_t shift) {
    return __SSAT(value >> (shift & 31U), sat);
}

__STATIC_FORCEINLINE uint32_t __USAT_ASR(int32_t value, uint32_t sat, uint32_t shift) {
    return __USAT(value >> (shift & 31U), sat);
}

__STATIC_FORCEINLINE int32_t __SSAT16(int32_t value, uint32_t sat) {
    int32_t hi = (int16_t) (value >> 16);
    int32_t lo = (int16_t) value;
    hi = __SSAT(hi, sat);
    lo = __SSAT(lo, sat);
    return (hi << 16) | (lo & 0xffff);
}

__STATIC_FORCEINLINE uint32_t __USAT16(int32_t value, uint32_t sat) {
    int32_t hi = (int16_t) (value >> 16);
    int32_t lo = (int16_t) value;
    uint32_t uhi = __USAT(hi, sat);
    uint32_t ulo = __USAT(lo, sat);
    return (uhi << 16) | (ulo & 0xffff);
}

__STATIC_FORCEINLINE uint32_t __QADD16(uint32_t value1, uint32_t value2) {
    int32_t hi = (int16_t) (value1 >> 16) + (int16_t) (value2 >> 16);
    int32_t lo = (int16_t) value1 + (int16_t) value2;
    hi = __SSAT(hi, 16);
    lo = __SSAT(lo, 16);
    return ((uint32_t) ((uint16_t) hi) << 16) | (uint16_t) lo;
}

__STATIC_FORCEINLINE uint32_t __QSUB16(uint32_t value1, uint32_t value2) {
    int32_t hi = (int16_t) (value1 >> 16) - (int16_t) (value2 >> 16);
    int32_t lo = (int16_t) value1 - (int16_t) value2;
    hi = __SSAT(hi, 16);
    lo = __SSAT(lo, 16);
    return ((uint32_t) ((uint16_t) hi) << 16) | (uint16_t) lo;
}

__STATIC_FORCEINLINE uint32_t __PKHBT(uint32_t value1, uint32_t value2, uint32_t shift) {
    return (value1 & 0x0000ffffU) | ((value2 << shift) & 0xffff0000U);
}

__STATIC_FORCEINLINE uint32_t __PKHTB(uint32_t value1, uint32_t value2, uint32_t shift) {
    return (value1 & 0xffff0000U) | ((value2 >> shift) & 0x0000ffffU);
}

__STATIC_FORCEINLINE int32_t __SMLAD(uint32_t value1, uint32_t value2, int32_t acc) {
    int32_t a0 = (int16_t) value1;
    int32_t a1 = (int16_t) (value1 >> 16);
    int32_t b0 = (int16_t) value2;
    int32_t b1 = (int16_t) (value2 >> 16);
    return acc + (a0 * b0) + (a1 * b1);
}

__STATIC_FORCEINLINE int32_t __SMLADX(uint32_t value1, uint32_t value2, int32_t acc) {
    int32_t a0 = (int16_t) value1;
    int32_t a1 = (int16_t) (value1 >> 16);
    int32_t b0 = (int16_t) value2;
    int32_t b1 = (int16_t) (value2 >> 16);
    return acc + (a0 * b1) + (a1 * b0);
}

__STATIC_FORCEINLINE int32_t __SMUAD(uint32_t value1, uint32_t value2) {
    return __SMLAD(value1, value2, 0);
}

__STATIC_FORCEINLINE int32_t __SMUADX(uint32_t value1, uint32_t value2) {
    return __SMLADX(value1, value2, 0);
}

__STATIC_FORCEINLINE uint32_t __USAD8(uint32_t value1, uint32_t value2) {
    uint32_t sum = 0;
    for (int i = 0; i < 4; i++) {
        int a = (value1 >> (i * 8)) & 0xff;
        int b = (value2 >> (i * 8)) & 0xff;
        sum += (uint32_t) ((a > b) ? (a - b) : (b - a));
    }
    return sum;
}

__STATIC_FORCEINLINE uint32_t __USADA8(uint32_t value1, uint32_t value2, uint32_t acc) {
    return acc + __USAD8(value1, value2);
}

#endif /* ESP_VISION_IMLIB_CMSIS_EXTENSION_H */
