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

#include <string.h>

#include "py/runtime.h"
#include "umm_malloc.h"

void umm_alloc_fail(void) {
    mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("umm_malloc failed"));
}

void umm_init_x(size_t size) {
    (void) size;
}

void *umm_malloc(size_t size) {
    return m_malloc(size);
}

void *umm_calloc(size_t num, size_t size) {
    void *ptr = m_malloc(num * size);
    memset(ptr, 0, num * size);
    return ptr;
}

void *umm_realloc(void *ptr, size_t size) {
    return m_realloc(ptr, size);
}

void umm_free(void *ptr) {
    m_free(ptr);
}
