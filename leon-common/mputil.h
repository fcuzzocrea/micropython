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

qstr mp_obj_fun_get_source(mp_const_obj_t fun_in);
size_t mp_code_state_get_line(const mp_code_state *code_state, qstr *source_file, qstr *block_name);
void mp_exec_str(const char *src, mp_parse_input_kind_t input_kind);
uint32_t mp_exec_mpy(const byte *buf, size_t len);
