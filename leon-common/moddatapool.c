/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2016 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Porting of
 * MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
 */

#include <stdio.h>

#include "py/stackctrl.h"
#include "py/runtime.h"
#include "py/gc.h"
#include "py/obj.h"
#include "moddatapool.h"

#define DATAPOOL_OBJ_TO_ID(o) ((uint32_t)MP_OBJ_TO_PTR(o))
#define DATAPOOL_OBJ_FROM_ID(id) (MP_OBJ_FROM_PTR((void*)(id)))

STATIC rtems_id datapool_sem;
STATIC uint32_t datapool_old_state;
STATIC mp_state_ctx_t datapool_state_ctx;

// these 2 function do a mini thread switch to the datapool state
STATIC void datapool_enter(void) {
    // obtain semaphore
    rtems_semaphore_obtain(datapool_sem, 0, 0);

    // save old thread state
    datapool_old_state = _Thread_Executing->Start.numeric_argument;

    // set the MicroPython context for this task
    _Thread_Executing->Start.numeric_argument = (uint32_t)&datapool_state_ctx;

    // we are on a foreign stack so need to set the stack top and limit
    mp_stack_ctrl_init();
    mp_stack_set_limit(2048);
}

STATIC void datapool_exit(void) {
    // restore old thread state
    _Thread_Executing->Start.numeric_argument = datapool_old_state;

    // release semaphore
    rtems_semaphore_release(datapool_sem);
}

void datapool_init(void *datapool_heap, size_t datapool_heap_size) {
    rtems_name name = rtems_build_name('M', 'P', 'D', 'P');
    rtems_status_code status = rtems_semaphore_create(name, 1, 0, 0, &datapool_sem);
    if (status != RTEMS_SUCCESSFUL) {
        mp_printf(&mp_plat_print, "datapool_init: error creating semaphore; %d\n", status);
        return;
    }

    datapool_enter();

    // initialise the heap
    gc_init(datapool_heap, datapool_heap + datapool_heap_size);

    // initialise the MicroPython runtime
    mp_init();

    // initialise the datapool index hash table
    mp_map_init(&MP_STATE_PORT(datapool_index), 0);

    datapool_exit();
}

void datapool_deinit(void) {
    datapool_enter();

    // deinitialise the MicroPython runtime
    mp_deinit();

    datapool_exit();
}

void datapool_heap_lock(void) {
    datapool_enter();
    gc_lock();
    datapool_exit();
}

void datapool_heap_unlock(void) {
    datapool_enter();
    gc_unlock();
    datapool_exit();
}

datapool_result_t datapool_create(const char *id, datapool_t *dp) {
    datapool_enter();
    datapool_result_t res = DATAPOOL_OK;

    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        qstr qst = qstr_from_str(id);
        mp_obj_t key = MP_OBJ_NEW_QSTR(qst);
        mp_map_elem_t *elem = mp_map_lookup(&MP_STATE_PORT(datapool_index), key, MP_MAP_LOOKUP);
        if (elem == NULL) {
            // a new datapool
            // Create the dict before adding to the hash table to keep the
            // table consistent in case memory allocation fails.
            mp_obj_t d = mp_obj_new_dict(0);
            mp_map_lookup(&MP_STATE_PORT(datapool_index), key, MP_MAP_LOOKUP_ADD_IF_NOT_FOUND)->value = d;
            *dp = DATAPOOL_OBJ_TO_ID(d);
        } else {
            // existing datapool with the same id
            res = DATAPOOL_ALREADY_EXISTS;
        }
        nlr_pop();
    } else {
        // exception, most likely a memory error
        res = DATAPOOL_MEMORY_ERROR;
    }

    datapool_exit();
    return res;
}

datapool_result_t datapool_ident(const char *id, datapool_t *dp) {
    datapool_enter();
    datapool_result_t res = DATAPOOL_OK;

    qstr qst = qstr_find_strn(id, strlen(id));
    if (qst == MP_QSTR_NULL) {
        res = DATAPOOL_NOT_FOUND;
    } else {
        mp_obj_t key = MP_OBJ_NEW_QSTR(qst);
        mp_map_elem_t *elem = mp_map_lookup(&MP_STATE_PORT(datapool_index), key, MP_MAP_LOOKUP);
        if (elem == NULL) {
            res = DATAPOOL_NOT_FOUND;
        } else {
            *dp = DATAPOOL_OBJ_TO_ID(elem->value);
        }
    }

    datapool_exit();
    return res;
}

datapool_result_t datapool_set_buf(datapool_t dp_in, int32_t idx, const void *data, size_t len) {
    datapool_enter();
    datapool_result_t res = DATAPOOL_OK;

    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        mp_obj_t dp = DATAPOOL_OBJ_FROM_ID(dp_in);
        mp_map_t *map = mp_obj_dict_get_map(dp);
        mp_obj_t key = MP_OBJ_NEW_SMALL_INT(idx);
        mp_map_elem_t *elem = mp_map_lookup(map, key, MP_MAP_LOOKUP);
        if (elem == NULL) {
            // a new entry
            // Create the bytearray before adding to the hash table to keep the
            // table consistent in case memory allocation fails.
            mp_obj_t ar = mp_obj_new_bytearray(len, (void*)data);
            mp_map_lookup(map, key, MP_MAP_LOOKUP_ADD_IF_NOT_FOUND)->value = ar;
        } else {
            // an existing entry
            mp_buffer_info_t bufinfo;
            if (!mp_get_buffer(elem->value, &bufinfo, MP_BUFFER_WRITE)) {
                // object in datapool is not a bytearray, internal error
                res = DATAPOOL_INTERNAL_ERROR;
            } else if (len != bufinfo.len) {
                // length of datapool entry does not match requested length
                res = DATAPOOL_ITEM_WRONG_SIZE;
            } else {
                // copy data into datapool
                memcpy(bufinfo.buf, data, len);
            }
        }
        nlr_pop();
    } else {
        // exception, most likely a memory error
        res = DATAPOOL_MEMORY_ERROR;
    }

    datapool_exit();
    return res;
}

datapool_result_t datapool_get_buf(datapool_t dp_in, int32_t idx, void *data, size_t len) {
    datapool_enter();
    datapool_result_t res = DATAPOOL_OK;

    mp_obj_t dp = DATAPOOL_OBJ_FROM_ID(dp_in);
    mp_map_t *map = mp_obj_dict_get_map(dp);
    mp_obj_t key = MP_OBJ_NEW_SMALL_INT(idx);
    mp_map_elem_t *elem = mp_map_lookup(map, key, MP_MAP_LOOKUP);
    if (elem == NULL) {
        // idx not found
        res = DATAPOOL_INDEX_NOT_FOUND;
    } else {
        // found
        mp_buffer_info_t bufinfo;
        if (!mp_get_buffer(elem->value, &bufinfo, MP_BUFFER_READ)) {
            // object in datapool is not a bytearray, internal error
            res = DATAPOOL_INTERNAL_ERROR;
        } else if (len != bufinfo.len) {
            // length of datapool entry does not match requested length
            res = DATAPOOL_ITEM_WRONG_SIZE;
        } else {
            // copy data out of datapool
            memcpy(data, bufinfo.buf, len);
        }
    }

    datapool_exit();
    return res;
}

datapool_result_t datapool_set_u32(datapool_t dp_in, int32_t idx, uint32_t val) {
    return datapool_set_buf(dp_in, idx, &val, sizeof(val));
}

datapool_result_t datapool_get_u32(datapool_t dp_in, int32_t idx, uint32_t *val) {
    return datapool_get_buf(dp_in, idx, val, sizeof(*val));
}

datapool_result_t datapool_set_double(datapool_t dp_in, int32_t idx, double val) {
    return datapool_set_buf(dp_in, idx, &val, sizeof(val));
}

datapool_result_t datapool_get_double(datapool_t dp_in, int32_t idx, double *val) {
    return datapool_get_buf(dp_in, idx, val, sizeof(*val));
}

/******************************************************************************/
// MicroPython bindings for datapool

typedef struct _datapool_error_t {
    const mp_obj_type_t *exc;
    const char *msg;
} datapool_error_t;

STATIC const datapool_error_t datapool_error_table[] = {
    [DATAPOOL_OK                ] = {NULL, "ok"},
    [DATAPOOL_NOT_FOUND         ] = {&mp_type_OSError, "datapool not found"},
    [DATAPOOL_ALREADY_EXISTS    ] = {&mp_type_OSError, "datapool already exists"},
    [DATAPOOL_INDEX_NOT_FOUND   ] = {&mp_type_KeyError, "datapool index not found"},
    [DATAPOOL_ITEM_WRONG_SIZE   ] = {&mp_type_ValueError, "datapool item wrong size"},
    [DATAPOOL_MEMORY_ERROR      ] = {&mp_type_MemoryError, "datapool memory error"},
    [DATAPOOL_INTERNAL_ERROR    ] = {&mp_type_RuntimeError, "datapool internal error"},
};

void datapool_result_check(datapool_result_t res) {
    if (res != DATAPOOL_OK) {
        nlr_raise(mp_obj_new_exception_msg(datapool_error_table[res].exc, datapool_error_table[res].msg));
    }
}

typedef struct _datapool_obj_t {
    mp_obj_base_t base;
    datapool_t dp;
} datapool_obj_t;

STATIC void datapool_obj_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    datapool_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "datapool(0x%08x)", (uint)self->dp);
}

STATIC mp_obj_t datapool_obj_set_buf(mp_obj_t self_in, mp_obj_t idx_in, mp_obj_t buf_in) {
    datapool_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf_in, &bufinfo, MP_BUFFER_READ);
    datapool_result_t res = datapool_set_buf(self->dp,
        mp_obj_get_int(idx_in), bufinfo.buf, bufinfo.len);
    datapool_result_check(res);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(datapool_obj_set_buf_obj, datapool_obj_set_buf);

STATIC mp_obj_t datapool_obj_get_buf(mp_obj_t self_in, mp_obj_t idx_in, mp_obj_t buf_in) {
    datapool_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf_in, &bufinfo, MP_BUFFER_WRITE);
    datapool_result_t res = datapool_get_buf(self->dp,
        mp_obj_get_int(idx_in), bufinfo.buf, bufinfo.len);
    datapool_result_check(res);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(datapool_obj_get_buf_obj, datapool_obj_get_buf);

STATIC mp_obj_t datapool_obj_set_u32(mp_obj_t self_in, mp_obj_t idx_in, mp_obj_t val_in) {
    datapool_obj_t *self = MP_OBJ_TO_PTR(self_in);
    datapool_result_t res = datapool_set_u32(self->dp,
        mp_obj_get_int(idx_in), mp_obj_get_int_truncated(val_in));
    datapool_result_check(res);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(datapool_obj_set_u32_obj, datapool_obj_set_u32);

STATIC mp_obj_t datapool_obj_get_u32(mp_obj_t self_in, mp_obj_t idx_in) {
    datapool_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint32_t val;
    datapool_result_t res = datapool_get_u32(self->dp, mp_obj_get_int(idx_in), &val);
    datapool_result_check(res);
    return mp_obj_new_int_from_uint(val);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(datapool_obj_get_u32_obj, datapool_obj_get_u32);

STATIC mp_obj_t datapool_obj_set_double(mp_obj_t self_in, mp_obj_t idx_in, mp_obj_t val_in) {
    datapool_obj_t *self = MP_OBJ_TO_PTR(self_in);
    datapool_result_t res = datapool_set_double(self->dp,
        mp_obj_get_int(idx_in), mp_obj_get_float(val_in));
    datapool_result_check(res);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(datapool_obj_set_double_obj, datapool_obj_set_double);

STATIC mp_obj_t datapool_obj_get_double(mp_obj_t self_in, mp_obj_t idx_in) {
    datapool_obj_t *self = MP_OBJ_TO_PTR(self_in);
    double val;
    datapool_result_t res = datapool_get_double(self->dp, mp_obj_get_int(idx_in), &val);
    datapool_result_check(res);
    return mp_obj_new_float(val);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(datapool_obj_get_double_obj, datapool_obj_get_double);

STATIC const mp_rom_map_elem_t datapool_obj_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_set_buf), MP_ROM_PTR(&datapool_obj_set_buf_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_buf), MP_ROM_PTR(&datapool_obj_get_buf_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_u32), MP_ROM_PTR(&datapool_obj_set_u32_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_u32), MP_ROM_PTR(&datapool_obj_get_u32_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_double), MP_ROM_PTR(&datapool_obj_set_double_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_double), MP_ROM_PTR(&datapool_obj_get_double_obj) },
};
STATIC MP_DEFINE_CONST_DICT(datapool_obj_locals_dict, datapool_obj_locals_dict_table);

STATIC const mp_obj_type_t datapool_obj_type = {
    { &mp_type_type },
    .name = MP_QSTR_datapool,
    .print = datapool_obj_print,
    .locals_dict = (void*)&datapool_obj_locals_dict,
};

STATIC mp_obj_t mod_datapool_heap_lock(void) {
    datapool_heap_lock();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(mod_datapool_heap_lock_obj, mod_datapool_heap_lock);

STATIC mp_obj_t mod_datapool_heap_unlock(void) {
    datapool_heap_unlock();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(mod_datapool_heap_unlock_obj, mod_datapool_heap_unlock);

STATIC mp_obj_t mod_datapool_create(mp_obj_t id_in) {
    const char *str = mp_obj_str_get_str(id_in);
    datapool_obj_t *dp = m_new_obj(datapool_obj_t);
    dp->base.type = &datapool_obj_type;
    datapool_result_t res = datapool_create(str, &dp->dp);
    datapool_result_check(res);
    return MP_OBJ_FROM_PTR(dp);
}
MP_DEFINE_CONST_FUN_OBJ_1(mod_datapool_create_obj, mod_datapool_create);

STATIC mp_obj_t mod_datapool_ident(mp_obj_t id_in) {
    const char *str = mp_obj_str_get_str(id_in);
    datapool_obj_t *dp = m_new_obj(datapool_obj_t);
    dp->base.type = &datapool_obj_type;
    datapool_result_t res = datapool_ident(str, &dp->dp);
    datapool_result_check(res);
    return MP_OBJ_FROM_PTR(dp);
}
MP_DEFINE_CONST_FUN_OBJ_1(mod_datapool_ident_obj, mod_datapool_ident);

STATIC const mp_rom_map_elem_t mp_module_datapool_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_datapool) },
    { MP_ROM_QSTR(MP_QSTR_heap_lock), MP_ROM_PTR(&mod_datapool_heap_lock_obj) },
    { MP_ROM_QSTR(MP_QSTR_heap_unlock), MP_ROM_PTR(&mod_datapool_heap_unlock_obj) },
    { MP_ROM_QSTR(MP_QSTR_create), MP_ROM_PTR(&mod_datapool_create_obj) },
    { MP_ROM_QSTR(MP_QSTR_ident), MP_ROM_PTR(&mod_datapool_ident_obj) },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_datapool_globals, mp_module_datapool_globals_table);

const mp_obj_module_t mp_module_datapool = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_datapool_globals,
};
