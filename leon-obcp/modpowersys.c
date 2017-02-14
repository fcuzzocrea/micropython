/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2016 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Porting of
 * MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
 */

#include <stdio.h>
#include <math.h>
#include "py/runtime.h"
#include "py/obj.h"
#include "obcp.h"

// read voltages into the given buffer
STATIC mp_obj_t powersys_read_voltages_into(mp_obj_t buf_in) {
    // get the input buffer
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf_in, &bufinfo, MP_BUFFER_WRITE);

    // check the buffer is the correct size
    if (bufinfo.len != N_VOLT * sizeof(double)) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "buffer wrong size"));
    }

    // store the voltages into the buffer
    double *buf = (double*)bufinfo.buf;
    double t = get_time();
    for (int i = 0; i < N_VOLT; ++i) {
        buf[i] = 12.34 + 0.5 * sin(t); // add some variation depending on the time
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(powersys_read_voltages_into_obj, powersys_read_voltages_into);

// read currents into the given buffer
STATIC mp_obj_t powersys_read_currents_into(mp_obj_t buf_in) {
    // get the input buffer
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf_in, &bufinfo, MP_BUFFER_WRITE);

    // check the buffer is the correct size
    if (bufinfo.len != N_CURR * sizeof(double)) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "buffer wrong size"));
    }

    // store the currents into the buffer
    double *buf = (double*)bufinfo.buf;
    double t = get_time();
    for (int i = 0; i < N_CURR; ++i) {
        buf[i] = 34.56e-3 + 1e-4 * sin(t); // add some variation depending on the time
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(powersys_read_currents_into_obj, powersys_read_currents_into);

// this table holds all the functions exported from this module
STATIC const mp_rom_map_elem_t powersys_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_mem) },
    { MP_ROM_QSTR(MP_QSTR_read_voltages_into), MP_ROM_PTR(&powersys_read_voltages_into_obj) },
    { MP_ROM_QSTR(MP_QSTR_read_currents_into), MP_ROM_PTR(&powersys_read_currents_into_obj) },
};

STATIC MP_DEFINE_CONST_DICT(powersys_module_globals, powersys_module_globals_table);

// this structure describes the actual "powersys" module
const mp_obj_module_t mp_module_powersys = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&powersys_module_globals,
};
