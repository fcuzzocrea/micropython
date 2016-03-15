/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2016 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Porting of
 * MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
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
    DATAPOOL_WRONG_SIZE,
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
