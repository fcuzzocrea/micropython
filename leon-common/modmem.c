/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2016 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Porting of
 * MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
 *
 * This "mem" module provides raw access to read/write arbitrary memory
 * locations.  It is simple and serves also as an example of how to write
 * a MicroPython C module.  Use it as follows:
 *
 *      import mem
 *
 *      val = mem.read_u32(0x40100000) # read 32-bit value at given location
 *      mem.write_u32(0x40100000, 0x12345678) # write 32-bit value
 *
 *      buf = bytearray(256)            # create a buffer of bytes
 *      mem.read_buf(0x40100000, buf)   # read 64 words (256 bytes) into buf
 *      mem.write_buf(0x40100000, buf)  # write buf to memory
 *
 *      import array
 *      ar = array.array('L', 8 * [0])  # create array of 8, 32-bit unsigned ints
 *      mem.read_buf(0x40100000, ar)    # read into the array
 *      for x in ar:
 *          print(x)                    # this will access ar as 32-bit uints
 */

#include <stdio.h>

#include "py/runtime.h"
#include "py/obj.h"

// read 32-bit value from the given address
STATIC mp_obj_t mem_read_u32(mp_obj_t addr_in) {
    // we use mp_obj_get_int_truncated to allow a full, unsigned 32-bit address
    // (normally we would use mp_obj_get_int to get a signed value)
    const uint32_t *src = (const uint32_t*)(uintptr_t)mp_obj_get_int_truncated(addr_in);

    // we want to return an unsigned integer, so use mp_obj_new_int_from_uint
    // (normally we would use mp_obj_new_int for a signed integer)
    return mp_obj_new_int_from_uint((mp_uint_t)*src);
}
MP_DEFINE_CONST_FUN_OBJ_1(mem_read_u32_obj, mem_read_u32);

// write 32-bit value to the given address
STATIC mp_obj_t mem_write_u32(mp_obj_t addr_in, mp_obj_t val_in) {
    uint32_t *dest = (uint32_t*)(uintptr_t)mp_obj_get_int_truncated(addr_in);
    *dest = mp_obj_get_int_truncated(val_in);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(mem_write_u32_obj, mem_write_u32);

// read memory region into the given buffer
STATIC mp_obj_t mem_read_buf(mp_obj_t addr_in, mp_obj_t buf_in) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf_in, &bufinfo, MP_BUFFER_WRITE);
    size_t n = bufinfo.len / 4;
    const uint32_t *src = (const uint32_t*)(uintptr_t)mp_obj_get_int_truncated(addr_in);
    uint32_t *dest = (uint32_t*)bufinfo.buf;
    while (n--) {
        *dest++ = *src++;
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(mem_read_buf_obj, mem_read_buf);

// write from the given buffer into memory region
STATIC mp_obj_t mem_write_buf(mp_obj_t addr_in, mp_obj_t buf_in) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf_in, &bufinfo, MP_BUFFER_READ);
    size_t n = bufinfo.len / 4;
    const uint32_t *src = (const uint32_t*)bufinfo.buf;
    uint32_t *dest = (uint32_t*)(uintptr_t)mp_obj_get_int_truncated(addr_in);
    while (n--) {
        *dest++ = *src++;
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(mem_write_buf_obj, mem_write_buf);

// this table holds all the functions exported from this module
STATIC const mp_rom_map_elem_t mem_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_mem) },
    { MP_ROM_QSTR(MP_QSTR_read_u32), MP_ROM_PTR(&mem_read_u32_obj) },
    { MP_ROM_QSTR(MP_QSTR_write_u32), MP_ROM_PTR(&mem_write_u32_obj) },
    { MP_ROM_QSTR(MP_QSTR_read_buf), MP_ROM_PTR(&mem_read_buf_obj) },
    { MP_ROM_QSTR(MP_QSTR_write_buf), MP_ROM_PTR(&mem_write_buf_obj) },
};

STATIC MP_DEFINE_CONST_DICT(mem_module_globals, mem_module_globals_table);

// this structure describes the actual "mem" module
const mp_obj_module_t mp_module_mem = {
    .base = { &mp_type_module },
    .name = MP_QSTR_mem,
    .globals = (mp_obj_dict_t*)&mem_module_globals,
};
