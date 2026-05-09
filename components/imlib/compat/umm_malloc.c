/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2007-2017 Ralph Hempel
 * Copyright (C) 2017-2024 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * UMM memory allocator.
 */

#include <stdint.h>
#include <string.h>

#include "esp_heap_caps.h"
#include "py/runtime.h"
#include "umm_malloc.h"

void umm_alloc_fail(void) {
    mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("umm_malloc failed"));
}

void umm_init_x(size_t size) {
    (void) size;
}

static void *umm_heap_malloc(size_t size) {
    void *ptr = heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (ptr == NULL) {
        ptr = heap_caps_malloc(size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    }
    if (ptr == NULL) {
        ptr = heap_caps_malloc(size, MALLOC_CAP_8BIT);
    }
    return ptr;
}

static void *umm_heap_realloc(void *ptr, size_t size) {
    void *new_ptr = heap_caps_realloc(ptr, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (new_ptr == NULL) {
        new_ptr = heap_caps_realloc(ptr, size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    }
    if (new_ptr == NULL) {
        new_ptr = heap_caps_realloc(ptr, size, MALLOC_CAP_8BIT);
    }
    return new_ptr;
}

void *umm_malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    return umm_heap_malloc(size);
}

void *umm_calloc(size_t num, size_t size) {
    if ((num == 0) || (size == 0)) {
        return NULL;
    }
    if (num > (SIZE_MAX / size)) {
        return NULL;
    }

    size_t total_size = num * size;
    void *ptr = umm_heap_malloc(total_size);
    if (ptr != NULL) {
        memset(ptr, 0, total_size);
    }
    return ptr;
}

void *umm_realloc(void *ptr, size_t size) {
    if (size == 0) {
        heap_caps_free(ptr);
        return NULL;
    }
    return umm_heap_realloc(ptr, size);
}

void umm_free(void *ptr) {
    heap_caps_free(ptr);
}
