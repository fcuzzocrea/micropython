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

#define MP_EXC_LOCATION_MSG_LEN (80)

// data structure used to return location of an exception
typedef struct _mp_exc_location_t {
    const char *filename; // always a valid string
    uint32_t line; // 0 if line numbers are not enabled
    const char *block; // NULL if block is unknown
    char exc_msg[MP_EXC_LOCATION_MSG_LEN]; // ASCIIZ string of the message
} mp_exc_location_t;

qstr mp_obj_fun_get_source(mp_const_obj_t fun_in);
size_t mp_code_state_get_line(const mp_code_state_t *code_state, qstr *source_file, qstr *block_name);
void mp_obj_exception_get_location(mp_obj_t exc, mp_exc_location_t *exc_loc);
void mp_exec_str(const char *src, mp_parse_input_kind_t input_kind);
mp_obj_t mp_exec_mpy_with_exc(const byte *buf, size_t len);
uint32_t mp_exec_mpy_with_exc_location(const byte *buf, size_t len, mp_exc_location_t *exc_loc);
uint32_t mp_exec_mpy(const byte *buf, size_t len);

void mp_mpy_modules_init(void);
void mp_mpy_modules_register(const byte *buf, size_t len);
