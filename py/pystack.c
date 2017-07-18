/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Damien P. George
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
 */

#include <stdio.h>

#include "py/runtime.h"
#include "py/pystack.h"

#if MICROPY_ENABLE_PYSTACK

// Enable this option to check that the amount of memory freed is equal to
// the last amount that was allocated.
#define CHECK_FREE_SIZE (0)

#define ALLOC_ALIGN (8)

void mp_pystack_init(void *start, void *end) {
    MP_STATE_THREAD(pystack_start) = start;
    MP_STATE_THREAD(pystack_end) = end;
    MP_STATE_THREAD(pystack_cur) = start;
}

void *mp_pystack_alloc(size_t n_bytes) {
    n_bytes = (n_bytes + (ALLOC_ALIGN - 1)) & ~(ALLOC_ALIGN - 1);
    #if CHECK_FREE_SIZE
    n_bytes += ALLOC_ALIGN;
    #endif
    if (MP_STATE_THREAD(pystack_cur) + n_bytes > MP_STATE_THREAD(pystack_end)) {
        // out of memory
        mp_exc_recursion_depth();
    }
    void *ptr = MP_STATE_THREAD(pystack_cur);
    MP_STATE_THREAD(pystack_cur) += n_bytes;
    #if CHECK_FREE_SIZE
    *(size_t*)(MP_STATE_THREAD(pystack_cur) - ALLOC_ALIGN) = n_bytes;
    #endif
    return ptr;
}

void mp_pystack_free(void *ptr) {
    #if CHECK_FREE_SIZE
    size_t n_bytes = MP_STATE_THREAD(pystack_cur) - (uint8_t*)ptr;
    if (n_bytes != *(size_t*)(MP_STATE_THREAD(pystack_cur) - ALLOC_ALIGN)) {
        printf("mp_pystack_free() failed: %u != %u\n", (uint)n_bytes,
            (uint)*(size_t*)(MP_STATE_THREAD(pystack_cur) - ALLOC_ALIGN));
        assert(0);
    }
    #endif
    MP_STATE_THREAD(pystack_cur) = ptr;
}

#endif
