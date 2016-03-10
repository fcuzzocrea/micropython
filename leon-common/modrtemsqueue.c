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
#include "modrtems.h"

typedef struct _rtems_queue_obj_t {
    mp_obj_base_t base;
    Objects_Id id;
} rtems_queue_obj_t;

STATIC void rtems_queue_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    rtems_queue_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "rtems_queue(0x%08x)", (uint)self->id);
}

STATIC mp_obj_t rtems_queue_delete(mp_obj_t self_in) {
    rtems_queue_obj_t *self = MP_OBJ_TO_PTR(self_in);
    rtems_status_code status = rtems_message_queue_delete(self->id);
    mod_rtems_status_code_check(status);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(rtems_queue_delete_obj, rtems_queue_delete);

typedef rtems_status_code (*queue_submit_t)(Objects_Id, void*, size_t);

STATIC mp_obj_t rtems_queue_submit(mp_obj_t self_in, mp_obj_t msg_in, queue_submit_t queue_submit) {
    rtems_queue_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(msg_in, &bufinfo, MP_BUFFER_READ);
    rtems_status_code status = queue_submit(self->id, bufinfo.buf, bufinfo.len);
    mod_rtems_status_code_check(status);
    return mp_const_none;
}

STATIC mp_obj_t rtems_queue_send(mp_obj_t self_in, mp_obj_t msg_in) {
    return rtems_queue_submit(self_in, msg_in, rtems_message_queue_send);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(rtems_queue_send_obj, rtems_queue_send);

STATIC mp_obj_t rtems_queue_urgent(mp_obj_t self_in, mp_obj_t msg_in) {
    return rtems_queue_submit(self_in, msg_in, rtems_message_queue_urgent);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(rtems_queue_urgent_obj, rtems_queue_urgent);

STATIC mp_obj_t rtems_queue_broadcast(mp_obj_t self_in, mp_obj_t msg_in) {
    rtems_queue_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(msg_in, &bufinfo, MP_BUFFER_READ);
    uint32_t count;
    rtems_status_code status = rtems_message_queue_broadcast(self->id, bufinfo.buf, bufinfo.len, &count);
    mod_rtems_status_code_check(status);
    return mp_obj_new_int(count);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(rtems_queue_broadcast_obj, rtems_queue_broadcast);

STATIC mp_obj_t rtems_queue_receive(mp_obj_t self_in, mp_obj_t msg_in) {
    // TODO we can't check that the buffer has enough room for the message
    // the caller must know the maximum size and pass in something big enough
    rtems_queue_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(msg_in, &bufinfo, MP_BUFFER_WRITE);
    uint32_t option_set = 0;
    rtems_interval timeout = 0;
    size_t sz = 0;
    rtems_status_code status = rtems_message_queue_receive(self->id, bufinfo.buf, &sz, option_set, timeout);
    mod_rtems_status_code_check(status);
    return MP_OBJ_NEW_SMALL_INT(sz);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(rtems_queue_receive_obj, rtems_queue_receive);

STATIC mp_obj_t rtems_queue_flush(mp_obj_t self_in) {
    rtems_queue_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint32_t count;
    rtems_status_code status = rtems_message_queue_flush(self->id, &count);
    mod_rtems_status_code_check(status);
    return mp_obj_new_int(count);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(rtems_queue_flush_obj, rtems_queue_flush);

STATIC mp_obj_t rtems_queue_get_number_pending(mp_obj_t self_in) {
    rtems_queue_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint32_t count;
    rtems_status_code status = rtems_message_queue_get_number_pending(self->id, &count);
    mod_rtems_status_code_check(status);
    return mp_obj_new_int(count);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(rtems_queue_get_number_pending_obj, rtems_queue_get_number_pending);

STATIC const mp_rom_map_elem_t rtems_queue_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_delete), MP_ROM_PTR(&rtems_queue_delete_obj) },
    { MP_ROM_QSTR(MP_QSTR_send), MP_ROM_PTR(&rtems_queue_send_obj) },
    { MP_ROM_QSTR(MP_QSTR_urgent), MP_ROM_PTR(&rtems_queue_urgent_obj) },
    { MP_ROM_QSTR(MP_QSTR_broadcast), MP_ROM_PTR(&rtems_queue_broadcast_obj) },
    { MP_ROM_QSTR(MP_QSTR_receive), MP_ROM_PTR(&rtems_queue_receive_obj) },
    { MP_ROM_QSTR(MP_QSTR_flush), MP_ROM_PTR(&rtems_queue_flush_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_number_pending), MP_ROM_PTR(&rtems_queue_get_number_pending_obj) },
};
STATIC MP_DEFINE_CONST_DICT(rtems_queue_locals_dict, rtems_queue_locals_dict_table);

STATIC const mp_obj_type_t rtems_queue_type = {
    { &mp_type_type },
    .name = MP_QSTR_queue,
    .print = rtems_queue_print,
    .locals_dict = (void*)&rtems_queue_locals_dict,
};

STATIC mp_obj_t mod_rtems_queue_create(mp_obj_t name_in, mp_obj_t max_size_in) {
    rtems_name name = mod_rtems_name_from_obj(name_in);
    mp_int_t max_size = mp_obj_get_int(max_size_in);
    rtems_queue_obj_t *self = m_new_obj(rtems_queue_obj_t);
    self->base.type = &rtems_queue_type;
    rtems_status_code status = rtems_message_queue_create(name, 4, max_size, RTEMS_DEFAULT_ATTRIBUTES, &self->id);
    mod_rtems_status_code_check(status);
    return MP_OBJ_FROM_PTR(self);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_rtems_queue_create_obj, mod_rtems_queue_create);

STATIC mp_obj_t mod_rtems_queue_ident(mp_obj_t name_in, mp_obj_t node_in) {
    rtems_name name = mod_rtems_name_from_obj(name_in);
    mp_int_t node = mp_obj_get_int(node_in);
    rtems_queue_obj_t *self = m_new_obj(rtems_queue_obj_t);
    self->base.type = &rtems_queue_type;
    rtems_status_code status = rtems_message_queue_ident(name, node, &self->id);
    mod_rtems_status_code_check(status);
    return MP_OBJ_FROM_PTR(self);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_rtems_queue_ident_obj, mod_rtems_queue_ident);

STATIC const mp_rom_map_elem_t mp_module_rtems_queue_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_queue) },

    { MP_ROM_QSTR(MP_QSTR_create), MP_ROM_PTR(&mod_rtems_queue_create_obj) },
    { MP_ROM_QSTR(MP_QSTR_ident), MP_ROM_PTR(&mod_rtems_queue_ident_obj) },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_rtems_queue_globals, mp_module_rtems_queue_globals_table);

const mp_obj_module_t mp_module_rtems_queue = {
    .base = { &mp_type_module },
    .name = MP_QSTR_queue,
    .globals = (mp_obj_dict_t*)&mp_module_rtems_queue_globals,
};
