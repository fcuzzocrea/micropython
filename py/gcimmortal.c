/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George
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

// This is a drop-in replacement for the standard memory manager which provides
// a simple immortal heap whereby only allocation is possible.  Allocation is
// implemented simply as incrementing a pointer to the heap.  There is no free,
// realloc or garbage collection.

#include <stdio.h>
#include <string.h>

#include "py/mpstate.h"
#include "py/gc.h"

#if MICROPY_ENABLE_GC && MICROPY_ENABLE_IMMORTAL_GC

#if 0 // print debugging info
#define DEBUG_PRINT (1)
#define DEBUG_printf printf
#else // don't print debugging info
#define DEBUG_PRINT (0)
#define DEBUG_printf(...) (void)0
#endif

// The granularity of the allocator.
// All returned pointers will be aligned to this size.
#define BYTE_GRANULARITY (sizeof(uint64_t))

// Each allocated block containts this header.
// It must fit into size BYTE_GRANULARITY.
typedef struct _block_header_t {
    size_t size;
} block_header_t;

void gc_init(void *start, void *end) {
    // align end pointer at byte granularity
    end = (void*)((uintptr_t)end & (~(BYTE_GRANULARITY - 1)));
    DEBUG_printf("Initializing GC heap: %p..%p = %u bytes\n", start, end, (uint)((byte*)end - (byte*)start));

    // set up parameters
    MP_STATE_MEM(gc_pool_start) = (byte*)start;
    MP_STATE_MEM(gc_pool_end) = (byte*)end;
    MP_STATE_MEM(gc_alloc_table_start) = (byte*)start; // pointer to next free location

    // zero out heap
    memset(start, 0, end - start);

    // unlock the GC
    MP_STATE_MEM(gc_lock_depth) = 0;

    // there is no collection
    MP_STATE_MEM(gc_auto_collect_enabled) = 0;
}

void gc_lock(void) {
    MP_STATE_MEM(gc_lock_depth)++;
}

void gc_unlock(void) {
    MP_STATE_MEM(gc_lock_depth)--;
}

bool gc_is_locked(void) {
    return MP_STATE_MEM(gc_lock_depth) != 0;
}

// ptr should be of type void*
#define VERIFY_PTR(ptr) ( \
        ((uintptr_t)(ptr) & (BYTE_GRANULARITY - 1)) == 0 /* must be aligned to granularity */ \
        && ptr >= (void*)MP_STATE_MEM(gc_pool_start)     /* must be above start of pool */ \
        && ptr < (void*)MP_STATE_MEM(gc_pool_end)        /* must be below end of pool */ \
    )

void gc_collect_start(void) {
    gc_lock();
}

void gc_collect_root(void **ptrs, size_t len) {
}

void gc_collect_end(void) {
    gc_unlock();
}

void gc_info(gc_info_t *info) {
    info->total = MP_STATE_MEM(gc_pool_end) - MP_STATE_MEM(gc_pool_start);
    info->used = MP_STATE_MEM(gc_alloc_table_start) - MP_STATE_MEM(gc_pool_start);
    info->free = MP_STATE_MEM(gc_pool_end) - MP_STATE_MEM(gc_alloc_table_start);
    info->num_1block = 0;
    info->num_2block = 0;
    info->max_block = 0;
}

void *gc_alloc(size_t n_bytes_in, bool has_finaliser) {
    // check if GC is locked
    if (MP_STATE_MEM(gc_lock_depth) > 0) {
        return NULL;
    }

    // check for 0 allocation
    if (n_bytes_in == 0) {
        return NULL;
    }

    // add room for block header
    size_t n_bytes = n_bytes_in + BYTE_GRANULARITY;

    // align number of bytes to heap granularity
    n_bytes = (n_bytes + BYTE_GRANULARITY - 1) & (~(BYTE_GRANULARITY - 1));

    DEBUG_printf("gc_alloc(%u bytes -> %u bytes)\n", (uint)n_bytes_in, (uint)n_bytes);

    // get pointer to first block
    byte *ret_ptr = MP_STATE_MEM(gc_alloc_table_start);

    // check we have enough room
    if (ret_ptr + n_bytes > MP_STATE_MEM(gc_pool_end)) {
        // not enough room left in heap
        return NULL;
    }

    // adnavce pointer
    MP_STATE_MEM(gc_alloc_table_start) += n_bytes;

    // store header
    block_header_t *hdr = (block_header_t*)ret_ptr;
    hdr->size = n_bytes;
    ret_ptr += BYTE_GRANULARITY;

    return ret_ptr;
}

void gc_free(void *ptr) {
    #if DEBUG_PRINT
    size_t n_bytes = gc_nbytes(ptr);
    DEBUG_printf("gc_free(%p, %u bytes)\n", ptr, (uint)n_bytes);
    #endif
}

size_t gc_nbytes(const void *ptr) {
    if (VERIFY_PTR(ptr)) {
        return ((block_header_t*)((const byte*)ptr - BYTE_GRANULARITY))->size - BYTE_GRANULARITY;
    }

    // invalid pointer
    return 0;
}

void *gc_realloc(void *ptr, size_t n_bytes, bool allow_move) {
    if (MP_STATE_MEM(gc_lock_depth) > 0) {
        return NULL;
    }

    // check for pure allocation
    if (ptr == NULL) {
        return gc_alloc(n_bytes, false);
    }

    // check for pure free
    if (n_bytes == 0) {
        gc_free(ptr);
        return NULL;
    }

    size_t n_existing = gc_nbytes(ptr);

    DEBUG_printf("gc_realloc(%p, %u existing bytes, %u wanted bytes)\n", ptr, n_existing, n_bytes);

    if (n_bytes <= n_existing) {
        // existing block contains enough bytes, so return it
        return ptr;
    } else if (allow_move) {
        // allocate a new block and copy existing data to it
        void *ptr_out = gc_alloc(n_bytes, false);
        if (ptr_out == NULL) {
            return ptr_out;
        }
        memcpy(ptr_out, ptr, n_existing);
        return ptr_out;
    } else {
        // can't move block so fail
        return NULL;
    }
}

void gc_dump_info(void) {
    gc_info_t info;
    gc_info(&info);
    mp_printf(&mp_plat_print, "GC: total: %u, used: %u, free: %u\n",
        (uint)info.total, (uint)info.used, (uint)info.free);
    mp_printf(&mp_plat_print, " No. of 1-blocks: %u, 2-blocks: %u, max blk sz: %u\n",
        (uint)info.num_1block, (uint)info.num_2block, (uint)info.max_block);
}

void gc_dump_alloc_table(void) {
    mp_printf(&mp_plat_print, "GC memory layout; from %p:", MP_STATE_MEM(gc_pool_start));
    mp_printf(&mp_plat_print, " no information available\n");
}

#endif // MICROPY_ENABLE_GC && MICROPY_ENABLE_IMMORTAL_GC
