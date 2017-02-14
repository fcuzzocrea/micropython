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
#include "py/obj.h"
#include "py/objint.h"
#include "modrtems.h"

STATIC const char *rtems_status_code_string[] = {
    [RTEMS_SUCCESSFUL              ] = "SUCCESSFUL",
    [RTEMS_TASK_EXITTED            ] = "TASK_EXITTED",
    [RTEMS_MP_NOT_CONFIGURED       ] = "MP_NOT_CONFIGURED",
    [RTEMS_INVALID_NAME            ] = "INVALID_NAME",
    [RTEMS_INVALID_ID              ] = "INVALID_ID",
    [RTEMS_TOO_MANY                ] = "TOO_MANY",
    [RTEMS_TIMEOUT                 ] = "TIMEOUT",
    [RTEMS_OBJECT_WAS_DELETED      ] = "OBJECT_WAS_DELETED",
    [RTEMS_INVALID_SIZE            ] = "INVALID_SIZE",
    [RTEMS_INVALID_ADDRESS         ] = "INVALID_ADDRESS",
    [RTEMS_INVALID_NUMBER          ] = "INVALID_NUMBER",
    [RTEMS_NOT_DEFINED             ] = "NOT_DEFINED",
    [RTEMS_RESOURCE_IN_USE         ] = "RESOURCE_IN_USE",
    [RTEMS_UNSATISFIED             ] = "UNSATISFIED",
    [RTEMS_INCORRECT_STATE         ] = "INCORRECT_STATE",
    [RTEMS_ALREADY_SUSPENDED       ] = "ALREADY_SUSPENDED",
    [RTEMS_ILLEGAL_ON_SELF         ] = "ILLEGAL_ON_SELF",
    [RTEMS_ILLEGAL_ON_REMOTE_OBJECT] = "ILLEGAL_ON_REMOTE_OBJECT",
    [RTEMS_CALLED_FROM_ISR         ] = "CALLED_FROM_ISR",
    [RTEMS_INVALID_PRIORITY        ] = "INVALID_PRIORITY",
    [RTEMS_INVALID_CLOCK           ] = "INVALID_CLOCK",
    [RTEMS_INVALID_NODE            ] = "INVALID_NODE",
    [RTEMS_NOT_CONFIGURED          ] = "NOT_CONFIGURED",
    [RTEMS_NOT_OWNER_OF_RESOURCE   ] = "NOT_OWNER_OF_RESOURCE",
    [RTEMS_NOT_IMPLEMENTED         ] = "NOT_IMPLEMENTED",
    [RTEMS_INTERNAL_ERROR          ] = "INTERNAL_ERROR",
    [RTEMS_NO_MEMORY               ] = "NO_MEMORY",
    [RTEMS_IO_ERROR                ] = "IO_ERROR",
    [RTEMS_PROXY_BLOCKING          ] = "PROXY_BLOCKING",
};

void mod_rtems_status_code_check(rtems_status_code status) {
    if (status != RTEMS_SUCCESSFUL) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError,
            rtems_status_code_string[status]));
    }
}

rtems_name mod_rtems_name_from_obj(mp_obj_t name_in) {
    mp_uint_t name_len;
    const char *name_str = mp_obj_str_get_data(name_in, &name_len);
    if (name_len != 4) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError,
            "name must be 4 bytes long"));
    }
    return rtems_build_name(name_str[0], name_str[1], name_str[2], name_str[3]);
}

// Return the task id of the script, starting from 0.
STATIC mp_obj_t mod_rtems_script_id(void) {
    return MP_STATE_PORT(rtems_script_id);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_rtems_script_id_obj, mod_rtems_script_id);

// this macro requires MPZ_DIG_SIZE=16
#define MPZ(val) \
    const mp_obj_int_t mpz_##val = \
        {{&mp_type_int}, {.neg=0, .fixed_dig=1, .alloc=2, .len=2, \
        .dig=(uint16_t[]){(val) & 0xffff, ((val) >> 16) & 0xffff}}};

STATIC MPZ(RTEMS_SEARCH_OTHER_NODES);
STATIC MPZ(RTEMS_SEARCH_LOCAL_NODE);

STATIC const mp_rom_map_elem_t mp_module_rtems_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_rtems) },

    { MP_ROM_QSTR(MP_QSTR_script_id), MP_ROM_PTR(&mod_rtems_script_id_obj) },

    { MP_ROM_QSTR(MP_QSTR_task), MP_ROM_PTR(&mp_module_rtems_task) },
    { MP_ROM_QSTR(MP_QSTR_queue), MP_ROM_PTR(&mp_module_rtems_queue) },
    { MP_ROM_QSTR(MP_QSTR_sem), MP_ROM_PTR(&mp_module_rtems_sem) },
    { MP_ROM_QSTR(MP_QSTR_timer), MP_ROM_PTR(&mp_module_rtems_timer) },

    { MP_ROM_QSTR(MP_QSTR_DEFAULT_ATTRIBUTES), MP_ROM_INT(RTEMS_DEFAULT_ATTRIBUTES) },
    { MP_ROM_QSTR(MP_QSTR_WAIT), MP_ROM_INT(RTEMS_WAIT) },
    { MP_ROM_QSTR(MP_QSTR_NO_WAIT), MP_ROM_INT(RTEMS_NO_WAIT) },
    { MP_ROM_QSTR(MP_QSTR_SEARCH_ALL_NODES), MP_ROM_INT(RTEMS_SEARCH_ALL_NODES) },
    { MP_ROM_QSTR(MP_QSTR_SEARCH_OTHER_NODES), MP_ROM_PTR(&mpz_RTEMS_SEARCH_OTHER_NODES) },
    { MP_ROM_QSTR(MP_QSTR_SEARCH_LOCAL_NODE), MP_ROM_PTR(&mpz_RTEMS_SEARCH_LOCAL_NODE) },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_rtems_globals, mp_module_rtems_globals_table);

const mp_obj_module_t mp_module_rtems = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_rtems_globals,
};
