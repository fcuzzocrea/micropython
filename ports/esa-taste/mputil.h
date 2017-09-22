/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2015-2016 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Porting of
 * MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
 */

#pragma once

#include "py/parse.h"
#include "py/bc.h"
#include "py/obj.h"

// data structure used to return location of an exception
typedef struct _mp_exc_location_t {
     const char *filename; // always a valid string
     uint32_t line; // 0 if line numbers are not enabled
     const char *block; // NULL if block is unknown
} mp_exc_location_t;

extern const mp_obj_type_t mp_type_mutable_attrtuple;

void mp_obj_exception_get_location(mp_obj_t exc, mp_exc_location_t *exc_loc);
void mp_exec_str(const char *src, mp_parse_input_kind_t input_kind);
mp_obj_t mp_exec_mpy_with_exc(const byte *buf, size_t len);
uint32_t mp_exec_mpy_with_exc_location(const byte *buf, size_t len, mp_exc_location_t *exc_loc);
uint32_t mp_exec_mpy(const byte *buf, size_t len);

void mp_mpy_modules_init(void);
void mp_mpy_modules_register(const byte *buf, size_t len);

/******************************************************************************/
// Interface of the Access type, used to store a reference to an object

#define MP_OBJ_ACCESS_INIT(obj) {{&mp_type_mutable_attrtuple}, 1, obj, MP_OBJ_FROM_PTR(mp_obj_access_fields)}

typedef struct _mp_obj_access_t {
    mp_obj_base_t base;
    size_t len;
    mp_obj_t items[2];
} mp_obj_access_t;

extern qstr mp_obj_access_fields[1];

mp_obj_t mp_obj_new_access(mp_obj_t arg);
MP_DECLARE_CONST_FUN_OBJ_1(mp_obj_new_access_obj);
mp_obj_t *mp_obj_access_get_items(mp_obj_t self);
