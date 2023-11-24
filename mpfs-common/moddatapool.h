/*
 * This file is part of the MicroPython port to MPFS platforms
 *
 * Based on LEON port originally made by George Robotics
 *
 * Copyright (c) 2023 Francescodario Cuzzocrea <bosconovic@gmail.com>
 */

#pragma once

// initialisation functions

void datapool_init(void *datapool_heap, size_t datapool_heap_size);
void datapool_deinit(void);

// C API
// note: idx into a datapool must fit in 31-bit signed integer
typedef enum {
    DATAPOOL_OK,
    DATAPOOL_NOT_FOUND,
    DATAPOOL_ALREADY_EXISTS,
    DATAPOOL_INDEX_NOT_FOUND,
    DATAPOOL_ITEM_WRONG_SIZE,
    DATAPOOL_MEMORY_ERROR,
    DATAPOOL_INTERNAL_ERROR,
} datapool_result_t;

typedef uint32_t datapool_t;

void datapool_heap_lock(void);
void datapool_heap_unlock(void);

datapool_result_t datapool_create(const char *id, datapool_t *dp);
datapool_result_t datapool_ident(const char *id, datapool_t *dp);
datapool_result_t datapool_set_buf(datapool_t dp_in, int32_t idx, const void *data, size_t len);
datapool_result_t datapool_get_buf(datapool_t dp_in, int32_t idx, void *data, size_t len);
datapool_result_t datapool_set_u32(datapool_t dp_in, int32_t idx, uint32_t val);
datapool_result_t datapool_get_u32(datapool_t dp_in, int32_t idx, uint32_t *val);
datapool_result_t datapool_set_double(datapool_t dp_in, int32_t idx, double val);
datapool_result_t datapool_get_double(datapool_t dp_in, int32_t idx, double *val);

// MicroPython module
extern const struct _mp_obj_module_t mp_module_datapool;
