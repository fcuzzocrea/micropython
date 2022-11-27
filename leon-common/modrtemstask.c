/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2015-2016 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Porting of
 * MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
 */

#include <stdio.h>
#include <rtems.h>

#include "py/nlr.h"
#include "py/runtime.h"
#include "modrtems.h"
#include "rtems_config.h"

#define BITS_PER_BYTE (8)

STATIC mp_obj_t mod_rtems_task_wake_when(mp_obj_t tod_in) {
    mp_obj_t *items;
    mp_obj_get_array_fixed_n(tod_in, 6, &items);
    rtems_time_of_day tod;
    tod.year = mp_obj_get_int(items[0]);
    tod.month = mp_obj_get_int(items[1]);
    tod.day = mp_obj_get_int(items[2]);
    tod.hour = mp_obj_get_int(items[3]);
    tod.minute = mp_obj_get_int(items[4]);
    tod.second = mp_obj_get_int(items[5]);
    rtems_status_code status = rtems_task_wake_when(&tod);
    mod_rtems_status_code_check(status);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rtems_task_wake_when_obj, mod_rtems_task_wake_when);

STATIC mp_obj_t mod_rtems_task_wake_after(mp_obj_t ticks_in) {
    rtems_status_code status = rtems_task_wake_after(mp_obj_get_int(ticks_in));
    mod_rtems_status_code_check(status);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rtems_task_wake_after_obj, mod_rtems_task_wake_after);

STATIC mp_obj_t mod_rtems_task_get_note(mp_obj_t note_id) {
    #if RTEMS_4
    uint32_t note = 0;
    rtems_status_code status = rtems_task_get_note(RTEMS_SELF,
        RTEMS_NOTEPAD_FIRST + mp_obj_get_int(note_id), &note);
    mod_rtems_status_code_check(status);
    return mp_obj_new_int_from_uint(note);
    #else
    mp_raise_NotImplementedError("get_note not supported");
    #endif
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rtems_task_get_note_obj, mod_rtems_task_get_note);

STATIC mp_obj_t mod_rtems_task_set_note(mp_obj_t note_id, mp_obj_t note_val) {
    #if RTEMS_4
    rtems_status_code status = rtems_task_set_note(RTEMS_SELF,
        RTEMS_NOTEPAD_FIRST + mp_obj_get_int(note_id), mp_obj_get_int_truncated(note_val));
    mod_rtems_status_code_check(status);
    return mp_const_none;
    #else
    mp_raise_NotImplementedError("set_note not supported");
    #endif
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_rtems_task_set_note_obj, mod_rtems_task_set_note);

STATIC mp_obj_t mod_rtems_task_get_affinity(void) {
    #if RTEMS_5 || RTEMS_6

    // Get the affinity.
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    rtems_status_code status = rtems_task_get_affinity(RTEMS_SELF, sizeof(cpu_set), &cpu_set);
    mod_rtems_status_code_check(status);

    // Convert the CPU set to a Python object (an integer).
    mp_uint_t set = 0;
    for (unsigned int i = 0; i < sizeof(cpu_set) * BITS_PER_BYTE; ++i) {
        if (CPU_ISSET(i, &cpu_set)) {
            set |= 1 << i;
        }
    }
    return mp_obj_new_int_from_uint(set);

    #else
    mp_raise_NotImplementedError("get_affinity not supported");
    #endif
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_rtems_task_get_affinity_obj, mod_rtems_task_get_affinity);

STATIC mp_obj_t mod_rtems_task_set_affinity(mp_obj_t set_in) {
    #if RTEMS_5 || RTEMS_6

    // Convert the Python object (an integer) to a CPU set.
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    mp_uint_t set = mp_obj_get_int(set_in);
    for (unsigned int i = 0; set; ++i, set >>= 1) {
        if (set & 1) {
            CPU_SET(i, &cpu_set);
        }
    }

    // Set the affinity.
    rtems_status_code status = rtems_task_set_affinity(RTEMS_SELF, sizeof(cpu_set), &cpu_set);
    mod_rtems_status_code_check(status);

    return mp_const_none;

    #else
    mp_raise_NotImplementedError("set_affinity not supported");
    #endif
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rtems_task_set_affinity_obj, mod_rtems_task_set_affinity);

STATIC const mp_rom_map_elem_t mp_module_rtems_task_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_task) },

    { MP_ROM_QSTR(MP_QSTR_wake_when), MP_ROM_PTR(&mod_rtems_task_wake_when_obj) },
    { MP_ROM_QSTR(MP_QSTR_wake_after), MP_ROM_PTR(&mod_rtems_task_wake_after_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_note), MP_ROM_PTR(&mod_rtems_task_get_note_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_note), MP_ROM_PTR(&mod_rtems_task_set_note_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_affinity), MP_ROM_PTR(&mod_rtems_task_get_affinity_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_affinity), MP_ROM_PTR(&mod_rtems_task_set_affinity_obj) },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_rtems_task_globals, mp_module_rtems_task_globals_table);

const mp_obj_module_t mp_module_rtems_task = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_rtems_task_globals,
};
