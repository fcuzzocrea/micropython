/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2015-2016 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Porting of
 * MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
 */

// Note: this module is a work-in-progress.  It currently does not work, the
// timers do not fire.

#include <stdio.h>
#include <rtems.h>

#include "py/nlr.h"
#include "py/obj.h"
#include "modrtems.h"

////////////////////////////////////////////////////////////////////////////////
// TIMER CLASS

typedef struct _mod_rtems_timer_obj_t {
    mp_obj_base_t base;
    Objects_Id id;
} mod_rtems_timer_obj_t;

STATIC void mod_rtems_timer_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    mod_rtems_timer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "rtems_timer(0x%08x)", (uint)self->id);
}

STATIC mp_obj_t mod_rtems_timer_cancel(mp_obj_t self_in) {
    mod_rtems_timer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    rtems_status_code status = rtems_timer_cancel(self->id);
    mod_rtems_status_code_check(status);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rtems_timer_cancel_obj, mod_rtems_timer_cancel);

STATIC mp_obj_t mod_rtems_timer_delete(mp_obj_t self_in) {
    mod_rtems_timer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    rtems_status_code status = rtems_timer_delete(self->id);
    mod_rtems_status_code_check(status);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rtems_timer_delete_obj, mod_rtems_timer_delete);

STATIC rtems_timer_service_routine f(rtems_id id, void* user_data) {
    printf("||| here %d %p\n", (int)id, user_data);
    ((byte*)user_data)[0] = 0;
}

STATIC mp_obj_t mod_rtems_timer_fire_after(mp_obj_t self_in, mp_obj_t ticks_in, mp_obj_t callback) {
    mod_rtems_timer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    rtems_interval ticks = mp_obj_get_int(ticks_in);
    rtems_status_code status = rtems_timer_fire_after(self->id, ticks, f, NULL);
    mod_rtems_status_code_check(status);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(mod_rtems_timer_fire_after_obj, mod_rtems_timer_fire_after);

STATIC mp_obj_t mod_rtems_timer_reset(mp_obj_t self_in) {
    mod_rtems_timer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    rtems_status_code status = rtems_timer_reset(self->id);
    mod_rtems_status_code_check(status);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rtems_timer_reset_obj, mod_rtems_timer_reset);

STATIC const mp_rom_map_elem_t mod_rtems_timer_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_cancel), MP_ROM_PTR(&mod_rtems_timer_cancel_obj) },
    { MP_ROM_QSTR(MP_QSTR_delete), MP_ROM_PTR(&mod_rtems_timer_delete_obj) },
    { MP_ROM_QSTR(MP_QSTR_fire_after), MP_ROM_PTR(&mod_rtems_timer_fire_after_obj) },
    { MP_ROM_QSTR(MP_QSTR_reset), MP_ROM_PTR(&mod_rtems_timer_reset_obj) },
};
STATIC MP_DEFINE_CONST_DICT(mod_rtems_timer_locals_dict, mod_rtems_timer_locals_dict_table);

STATIC const mp_obj_type_t mod_rtems_timer_type = {
    { &mp_type_type },
    .name = MP_QSTR_timer,
    .print = mod_rtems_timer_print,
    .locals_dict = (void*)&mod_rtems_timer_locals_dict,
};

////////////////////////////////////////////////////////////////////////////////
// TIMER MODULE

STATIC mp_obj_t mod_rtems_timer_create(mp_obj_t name_in) {
    rtems_name name = mod_rtems_name_from_obj(name_in);
    mod_rtems_timer_obj_t *self = m_new_obj(mod_rtems_timer_obj_t);
    self->base.type = &mod_rtems_timer_type;
    rtems_status_code status = rtems_timer_create(name, &self->id);
    mod_rtems_status_code_check(status);
    return MP_OBJ_FROM_PTR(self);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rtems_timer_create_obj, mod_rtems_timer_create);

STATIC mp_obj_t mod_rtems_timer_ident(mp_obj_t name_in) {
    rtems_name name = mod_rtems_name_from_obj(name_in);
    mod_rtems_timer_obj_t *self = m_new_obj(mod_rtems_timer_obj_t);
    self->base.type = &mod_rtems_timer_type;
    rtems_status_code status = rtems_timer_ident(name, &self->id);
    mod_rtems_status_code_check(status);
    return MP_OBJ_FROM_PTR(self);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_rtems_timer_ident_obj, mod_rtems_timer_ident);

STATIC const mp_rom_map_elem_t mp_module_rtems_timer_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_timer) },

    { MP_ROM_QSTR(MP_QSTR_create), MP_ROM_PTR(&mod_rtems_timer_create_obj) },
    { MP_ROM_QSTR(MP_QSTR_ident), MP_ROM_PTR(&mod_rtems_timer_ident_obj) },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_rtems_timer_globals, mp_module_rtems_timer_globals_table);

const mp_obj_module_t mp_module_rtems_timer = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_rtems_timer_globals,
};
