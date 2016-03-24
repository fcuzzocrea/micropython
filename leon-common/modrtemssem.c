/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2015-2016 George Robotics Limited
 * Copyright (c) 2016 David Sanchez de la Llana
 *
 * Provided to the European Space Agency as part of the project "Porting of
 * MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
 */

#include <stdio.h>
#include <rtems.h>

#include "py/nlr.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "modrtems.h"

typedef struct _rtems_sem_obj_t {
    mp_obj_base_t base;
    rtems_id id;
} rtems_sem_obj_t;

STATIC void rtems_sem_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    rtems_sem_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "rtems_sem(0x%08x)", (uint)self->id);
}

STATIC mp_obj_t rtems_sem_delete(mp_obj_t self_in) {
    rtems_sem_obj_t *self = MP_OBJ_TO_PTR(self_in);
    rtems_status_code status = rtems_semaphore_delete(self->id);
    mod_rtems_status_code_check(status);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(rtems_sem_delete_obj, rtems_sem_delete);

STATIC mp_obj_t rtems_sem_obtain(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_option, ARG_timeout };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_option, MP_ARG_INT, {.u_int = RTEMS_WAIT} },
        { MP_QSTR_timeout, MP_ARG_INT, {.u_int = 0} },
    };

    // parse args
    rtems_sem_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // extract arg values
    uint32_t option_set = args[ARG_option].u_int;
    rtems_interval timeout = args[ARG_timeout].u_int;

    // perform the operation
    rtems_status_code status = rtems_semaphore_obtain(self->id, option_set, timeout);
    mod_rtems_status_code_check(status);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(rtems_sem_obtain_obj, 1, rtems_sem_obtain);

STATIC mp_obj_t rtems_sem_release(mp_obj_t self_in) {
    rtems_sem_obj_t *self = MP_OBJ_TO_PTR(self_in);
    rtems_status_code status = rtems_semaphore_release(self->id);
    mod_rtems_status_code_check(status);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(rtems_sem_release_obj, rtems_sem_release);

STATIC mp_obj_t rtems_sem_flush(mp_obj_t self_in) {
    rtems_sem_obj_t *self = MP_OBJ_TO_PTR(self_in);
    rtems_status_code status = rtems_semaphore_flush(self->id);
    mod_rtems_status_code_check(status);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(rtems_sem_flush_obj, rtems_sem_flush);

// these are the methods for the sem object
STATIC const mp_rom_map_elem_t rtems_sem_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_delete), MP_ROM_PTR(&rtems_sem_delete_obj) },
    { MP_ROM_QSTR(MP_QSTR_obtain), MP_ROM_PTR(&rtems_sem_obtain_obj) },
    { MP_ROM_QSTR(MP_QSTR_release), MP_ROM_PTR(&rtems_sem_release_obj) },
    { MP_ROM_QSTR(MP_QSTR_flush), MP_ROM_PTR(&rtems_sem_flush_obj) },
};
STATIC MP_DEFINE_CONST_DICT(rtems_sem_locals_dict, rtems_sem_locals_dict_table);

// this is the sem object type
STATIC const mp_obj_type_t rtems_sem_type = {
    { &mp_type_type },
    .name = MP_QSTR_sem,
    .print = rtems_sem_print,
    .locals_dict = (mp_obj_dict_t*)&rtems_sem_locals_dict,
};

STATIC mp_obj_t mod_rtems_sem_create(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_name, ARG_count, ARG_attr, ARG_prio };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_name, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_count, MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_attr, MP_ARG_INT, {.u_int = RTEMS_DEFAULT_ATTRIBUTES} },
        { MP_QSTR_prio, MP_ARG_INT, {.u_int = 0} },
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // extract arg values
    rtems_name name = mod_rtems_name_from_obj(args[ARG_name].u_obj);
    uint32_t count = args[ARG_count].u_int;
    rtems_attribute attr = args[ARG_attr].u_int;
    rtems_task_priority prio = args[ARG_prio].u_int;

    // create sem object
    rtems_sem_obj_t *self = m_new_obj(rtems_sem_obj_t);
    self->base.type = &rtems_sem_type;
    rtems_status_code status = rtems_semaphore_create(name, count, attr, prio, &self->id);
    mod_rtems_status_code_check(status);
    return MP_OBJ_FROM_PTR(self);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(mod_rtems_sem_create_obj, 0, mod_rtems_sem_create);

STATIC mp_obj_t mod_rtems_sem_ident(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_name, ARG_node };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_name, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_node, MP_ARG_INT, {.u_int = RTEMS_SEARCH_ALL_NODES} },
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // extract arg values
    rtems_name name = mod_rtems_name_from_obj(args[ARG_name].u_obj);
    uint32_t node = args[ARG_node].u_int;

    // create sem object
    rtems_sem_obj_t *self = m_new_obj(rtems_sem_obj_t);
    self->base.type = &rtems_sem_type;
    rtems_status_code status = rtems_semaphore_ident(name, node, &self->id);
    mod_rtems_status_code_check(status);
    return MP_OBJ_FROM_PTR(self);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(mod_rtems_sem_ident_obj, 0, mod_rtems_sem_ident);

// these are the "static" functions available in the rtems.sem module
STATIC const mp_rom_map_elem_t mp_module_rtems_sem_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_sem) },
    { MP_ROM_QSTR(MP_QSTR_create), MP_ROM_PTR(&mod_rtems_sem_create_obj) },
    { MP_ROM_QSTR(MP_QSTR_ident), MP_ROM_PTR(&mod_rtems_sem_ident_obj) },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_rtems_sem_globals, mp_module_rtems_sem_globals_table);

// this is the rtems.sem module
const mp_obj_module_t mp_module_rtems_sem = {
    .base = { &mp_type_module },
    .name = MP_QSTR_sem,
    .globals = (mp_obj_dict_t*)&mp_module_rtems_sem_globals,
};
