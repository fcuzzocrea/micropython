/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2015-2016 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Porting of
 * MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
 */

#include <stdio.h>
#include <string.h>

#include "py/nlr.h"
#include "py/obj.h"
#include "modrtems.h"

// localtime()
// Return a 6-tuple containing: (year, month, mday, hour, minute, second)
// month   is 1-12
// mday    is 1-31
// hour    is 0-23
// minute  is 0-59
// second  is 0-59
STATIC mp_obj_t time_localtime(void) {
    rtems_time_of_day tod;
    rtems_status_code status = rtems_clock_get(RTEMS_CLOCK_GET_TOD, &tod);
    mod_rtems_status_code_check(status);
    mp_obj_t tuple[6] = {
        mp_obj_new_int(tod.year),
        mp_obj_new_int(tod.month),
        mp_obj_new_int(tod.day),
        mp_obj_new_int(tod.hour),
        mp_obj_new_int(tod.minute),
        mp_obj_new_int(tod.second),
    };
    return mp_obj_new_tuple(6, tuple);
}
MP_DEFINE_CONST_FUN_OBJ_0(time_localtime_obj, time_localtime);

// localtime_into(lst)
// Store into the given list: (year, month, mday, hour, minute, second)
// month   is 1-12
// mday    is 1-31
// hour    is 0-23
// minute  is 0-59
// second  is 0-59
STATIC mp_obj_t time_localtime_into(mp_obj_t lst) {
    if (!MP_OBJ_IS_TYPE(lst, &mp_type_list)) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "expecting a list"));
    }
    mp_obj_t *items;
    mp_obj_get_array_fixed_n(lst, 6, &items);
    rtems_time_of_day tod;
    rtems_status_code status = rtems_clock_get(RTEMS_CLOCK_GET_TOD, &tod);
    mod_rtems_status_code_check(status);
    items[0] = mp_obj_new_int(tod.year);
    items[1] = mp_obj_new_int(tod.month);
    items[2] = mp_obj_new_int(tod.day);
    items[3] = mp_obj_new_int(tod.hour);
    items[4] = mp_obj_new_int(tod.minute);
    items[5] = mp_obj_new_int(tod.second);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(time_localtime_into_obj, time_localtime_into);

// Returns the number of seconds since the Epoch, as a float.
// Has sub-second precision.
STATIC mp_obj_t time_time(void) {
    #if RTEMS_4_8
    rtems_clock_time_value t;
    rtems_status_code status = rtems_clock_get(RTEMS_CLOCK_GET_TIME_VALUE, &t);
    mod_rtems_status_code_check(status);
    return mp_obj_new_float(t.seconds + 1e-6 * t.microseconds);
    #else
    struct timeval tv;
    rtems_status_code status = rtems_clock_get_tod_timeval(&tv);
    mod_rtems_status_code_check(status);
    return mp_obj_new_float(tv.tv_sec + (mp_float_t)tv.tv_usec / 1000000);
    #endif
}
MP_DEFINE_CONST_FUN_OBJ_0(time_time_obj, time_time);

// Sleep for the given number of seconds.
// Seconds can be a float to sleep for a fractional number of seconds.
STATIC mp_obj_t time_sleep(mp_obj_t sec_in) {
    rtems_interval ticks_per_second;
    rtems_status_code status = rtems_clock_get(RTEMS_CLOCK_GET_TICKS_PER_SECOND, &ticks_per_second);
    mod_rtems_status_code_check(status);
    if (MP_OBJ_IS_INT(sec_in)) {
        status = rtems_task_wake_after(mp_obj_get_int(sec_in) * ticks_per_second);
    } else {
        status = rtems_task_wake_after(mp_obj_get_float(sec_in) * ticks_per_second);
    }
    mod_rtems_status_code_check(status);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(time_sleep_obj, time_sleep);

STATIC const mp_rom_map_elem_t time_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_time) },

    { MP_ROM_QSTR(MP_QSTR_localtime), MP_ROM_PTR(&time_localtime_obj) },
    { MP_ROM_QSTR(MP_QSTR_localtime_into), MP_ROM_PTR(&time_localtime_into_obj) },
    { MP_ROM_QSTR(MP_QSTR_time), MP_ROM_PTR(&time_time_obj) },
    { MP_ROM_QSTR(MP_QSTR_sleep), MP_ROM_PTR(&time_sleep_obj) },
};

STATIC MP_DEFINE_CONST_DICT(time_module_globals, time_module_globals_table);

const mp_obj_module_t mp_module_time = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&time_module_globals,
};
