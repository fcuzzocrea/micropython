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
#include "py/runtime.h"
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

STATIC mp_obj_t rtems_queue_submit(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args, queue_submit_t queue_submit) {
    enum { ARG_buf, ARG_option, ARG_timeout };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_buf, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_option, MP_ARG_INT, {.u_int = RTEMS_NO_WAIT} },
        { MP_QSTR_timeout, MP_ARG_INT, {.u_int = 0} },
    };

    // parse args
    rtems_queue_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[ARG_buf].u_obj, &bufinfo, MP_BUFFER_READ);
    rtems_status_code status = queue_submit(self->id, bufinfo.buf, bufinfo.len);

    // MicroPython extension to RTEMS API: allow to block with a timeout
    // timeout<=0 means wait forever
    if (args[ARG_option].u_int == RTEMS_WAIT && status == RTEMS_TOO_MANY) {
        mp_int_t t = 0;
        do {
            rtems_task_wake_after(1);
            status = queue_submit(self->id, bufinfo.buf, bufinfo.len);
            t += 1;
            if (args[ARG_timeout].u_int > 0 && t >= args[ARG_timeout].u_int) {
                status = RTEMS_TIMEOUT;
            }
        } while (status == RTEMS_TOO_MANY);
    }

    mod_rtems_status_code_check(status);
    return mp_const_none;
}

STATIC mp_obj_t rtems_queue_send(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    return rtems_queue_submit(n_args, pos_args, kw_args, rtems_message_queue_send);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(rtems_queue_send_obj, 1, rtems_queue_send);

STATIC mp_obj_t rtems_queue_urgent(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    return rtems_queue_submit(n_args, pos_args, kw_args, rtems_message_queue_urgent);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(rtems_queue_urgent_obj, 1, rtems_queue_urgent);

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

STATIC mp_obj_t rtems_queue_receive(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_buf, ARG_option, ARG_timeout };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_buf, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_option, MP_ARG_INT, {.u_int = RTEMS_WAIT} },
        { MP_QSTR_timeout, MP_ARG_INT, {.u_int = 0} },
    };

    // parse args
    rtems_queue_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[ARG_buf].u_obj, &bufinfo, MP_BUFFER_WRITE);
    uint32_t option_set = args[ARG_option].u_int;
    rtems_interval timeout = args[ARG_timeout].u_int;
    size_t sz = 0;
    rtems_status_code status = rtems_message_queue_receive(self->id, bufinfo.buf, &sz, option_set, timeout);
    if (sz > bufinfo.len) {
        // We can't check that the buffer has enough room for the message, so
        // the caller must know the maximum size and pass in something big enough.
        // Here we just do a sanity check and raise an exception if the buffer was
        // not large enough.
        nlr_raise(mp_obj_new_exception_msg(&mp_type_RuntimeError,
            "input buffer was too small for received message, heap is likely now corrupt!"));
    }
    mod_rtems_status_code_check(status);
    return MP_OBJ_NEW_SMALL_INT(sz);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(rtems_queue_receive_obj, 1, rtems_queue_receive);

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

STATIC mp_obj_t mod_rtems_queue_create(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_name, ARG_count, ARG_max_size, ARG_attr };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_name, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_count, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_max_size, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_attr, MP_ARG_INT, {.u_int = RTEMS_DEFAULT_ATTRIBUTES} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    rtems_name name = mod_rtems_name_from_obj(args[ARG_name].u_obj);
    mp_int_t count = args[ARG_count].u_int;
    mp_int_t max_size = args[ARG_max_size].u_int;
    uint32_t attr = args[ARG_attr].u_int;
    rtems_queue_obj_t *self = m_new_obj(rtems_queue_obj_t);
    self->base.type = &rtems_queue_type;
    rtems_status_code status = rtems_message_queue_create(name, count, max_size, attr, &self->id);
    mod_rtems_status_code_check(status);
    return MP_OBJ_FROM_PTR(self);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(mod_rtems_queue_create_obj, 0, mod_rtems_queue_create);

STATIC mp_obj_t mod_rtems_queue_ident(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_name, ARG_node };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_name, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_node, MP_ARG_INT, {.u_int = RTEMS_SEARCH_ALL_NODES} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    rtems_name name = mod_rtems_name_from_obj(args[ARG_name].u_obj);
    uint32_t node = args[ARG_node].u_int;
    rtems_queue_obj_t *self = m_new_obj(rtems_queue_obj_t);
    self->base.type = &rtems_queue_type;
    rtems_status_code status = rtems_message_queue_ident(name, node, &self->id);
    mod_rtems_status_code_check(status);
    return MP_OBJ_FROM_PTR(self);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(mod_rtems_queue_ident_obj, 0, mod_rtems_queue_ident);

STATIC const mp_rom_map_elem_t mp_module_rtems_queue_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_queue) },

    { MP_ROM_QSTR(MP_QSTR_create), MP_ROM_PTR(&mod_rtems_queue_create_obj) },
    { MP_ROM_QSTR(MP_QSTR_ident), MP_ROM_PTR(&mod_rtems_queue_ident_obj) },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_rtems_queue_globals, mp_module_rtems_queue_globals_table);

const mp_obj_module_t mp_module_rtems_queue = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_rtems_queue_globals,
};
