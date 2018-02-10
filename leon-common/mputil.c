/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2015-2016 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Porting of
 * MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
 */

#include "py/nlr.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/objfun.h"
#include "py/bc.h"
#include "py/builtin.h"
#include "py/gc.h"
#include "py/stackctrl.h"
#include "py/persistentcode.h"
#include "py/mphal.h"
#include "leon-common/mputil.h"

// Get the source file of a given function object.
qstr mp_obj_fun_get_source(mp_const_obj_t fun_in) {
    const mp_obj_fun_bc_t *fun = MP_OBJ_TO_PTR(fun_in);
    const byte *bc = fun->bytecode;
    mp_decode_uint(&bc); // skip n_state
    mp_decode_uint(&bc); // skip n_exc_stack
    bc++; // skip scope_params
    bc++; // skip n_pos_args
    bc++; // skip n_kwonly_args
    bc++; // skip n_def_pos_args

    mp_decode_uint(&bc); // skip code_info_size entry
    #if MICROPY_PERSISTENT_CODE
    return bc[2] | (bc[3] << 8);
    #else
    mp_decode_uint(&bc); // skip simple name
    return mp_decode_uint(&bc);
    #endif
}

// Get the line number info of some currently-executing bytecode.
size_t mp_code_state_get_line(const mp_code_state_t *code_state, qstr *source_file, qstr *block_name) {
    const byte *ip = code_state->fun_bc->bytecode;
    mp_decode_uint(&ip); // skip n_state
    mp_decode_uint(&ip); // skip n_exc_stack
    ip++; // skip scope_params
    ip++; // skip n_pos_args
    ip++; // skip n_kwonly_args
    ip++; // skip n_def_pos_args
    size_t bc = code_state->ip - ip;
    size_t code_info_size = mp_decode_uint(&ip);
    bc -= code_info_size;
    #if MICROPY_PERSISTENT_CODE
    *block_name = ip[0] | (ip[1] << 8);
    *source_file = ip[2] | (ip[3] << 8);
    ip += 4;
    #else
    *block_name = mp_decode_uint(&ip);
    *source_file = mp_decode_uint(&ip);
    #endif
    size_t source_line = 1;
    size_t c;
    while ((c = *ip)) {
        mp_uint_t b, l;
        if ((c & 0x80) == 0) {
            // 0b0LLBBBBB encoding
            b = c & 0x1f;
            l = c >> 5;
            ip += 1;
        } else {
            // 0b1LLLBBBB 0bLLLLLLLL encoding (l's LSB in second byte)
            b = c & 0xf;
            l = ((c << 4) & 0x700) | ip[1];
            ip += 2;
        }
        if (bc >= b) {
            bc -= b;
            source_line += l;
        } else {
            // found source line corresponding to bytecode offset
            break;
        }
    }
    return source_line;
}

// The following struct and function implement a simple printer that truncates the
// output data if it doesn't fit in the given buffer, leaving room for a null byte.

struct _exc_printer_t {
    size_t alloc;
    size_t len;
    char *buf;
};

STATIC void exc_add_strn(void *data, const char *str, size_t len) {
    struct _exc_printer_t *pr = data;
    if (pr->len + len >= pr->alloc) {
        // Not enough room so truncate data (and leave room for a null byte)
        len = pr->alloc - pr->len - 1;
    }
    memcpy(pr->buf + pr->len, str, len);
    pr->len += len;
}

// Get the location of an exception from the traceback data of the object.
void mp_obj_exception_get_location(mp_obj_t exc, mp_exc_location_t *exc_loc) {
    exc_loc->filename = "unknown";
    exc_loc->line = 0;
    exc_loc->block = NULL;
    if (mp_obj_is_exception_instance(exc)) {
        size_t n, *values;
        mp_obj_exception_get_traceback(exc, &n, &values);
        if (n >= 3) {
            exc_loc->filename = qstr_str(values[0]);
            #if MICROPY_ENABLE_SOURCE_LINE
            exc_loc->line = values[1];
            #endif
            if (values[2] != MP_QSTR_NULL) {
                exc_loc->block = qstr_str(values[2]);
            }
        }
    }

    // Render the exception message into the given buffer
    struct _exc_printer_t printer = {sizeof(exc_loc->exc_msg), 0, &exc_loc->exc_msg[0]};
    mp_print_t print = {&printer, exc_add_strn};
    mp_obj_print_helper(&print, exc, PRINT_EXC);
    exc_loc->exc_msg[printer.len] = '\0'; // add null terminating byte
}

#if MICROPY_ENABLE_COMPILER
// Execute a Python script passed a a string.
void mp_exec_str(const char *src, mp_parse_input_kind_t input_kind) {
    mp_lexer_t *lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, src, strlen(src), 0);
    if (lex == NULL) {
        mp_printf(&mp_plat_print, "MemoryError: lexer could not allocate memory\n");
        return;
    }

    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        qstr source_name = lex->source_name;
        mp_parse_tree_t parse_tree = mp_parse(lex, input_kind);
        mp_obj_t module_fun = mp_compile(&parse_tree, source_name, MP_EMIT_OPT_NONE, true);
        mp_call_function_0(module_fun);
        nlr_pop();
    } else {
        // uncaught exception
        mp_obj_print_exception(&mp_plat_print, MP_OBJ_FROM_PTR(nlr.ret_val));
    }
}
#endif

// Check an exception for SystemExit and extract its return value.
STATIC uint32_t mp_obj_exception_check_system_exit(mp_obj_t exc) {
    if (mp_obj_is_subclass_fast(MP_OBJ_FROM_PTR(mp_obj_get_type(exc)),
        MP_OBJ_FROM_PTR(&mp_type_SystemExit))) {
        // None is an exit value of 0; an int is its value; anything else is 1
        mp_obj_t exit_val = mp_obj_exception_get_value(exc);
        mp_int_t val = 0;
        if (exit_val != mp_const_none && !mp_obj_get_int_maybe(exit_val, &val)) {
            val = 1;
        }
        return 0x80 | (val & 255);
    } else {
        // any other exception
        return 0;
    }
}

// Execute a Python script passed as pre-compiled bytecode in a buffer.
// Returns MP_OBJ_NULL for no exception, otherwise the exception instance.
mp_obj_t mp_exec_mpy_with_exc(const byte *buf, size_t len) {
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        mp_store_global(MP_QSTR___file__, MP_OBJ_NEW_QSTR(MP_QSTR___main__));
        mp_raw_code_t *raw_code = mp_raw_code_load_mem(buf, len);
        mp_obj_t f = mp_make_function_from_raw_code(raw_code, MP_OBJ_NULL, MP_OBJ_NULL);
        mp_call_function_0(f);
        nlr_pop();
        // no exception was raised
        return MP_OBJ_NULL;
    } else {
        // uncaught exception
        return MP_OBJ_FROM_PTR(nlr.ret_val);
    }
}

// Execute a Python script passed as pre-compiled bytecode in a buffer.
// If an exception is raised then the location of the innermost part of the
// script that caused the exception is stored in exc_loc.
// Returns an exit code:
//  - 0 for normal exit
//  - 1 if a non-SystemExit exception was raised
//  - 128 or'd with return code if a SystemExit exception was raised
uint32_t mp_exec_mpy_with_exc_location(const byte *buf, size_t len, mp_exc_location_t *exc_loc) {
    mp_obj_t exc = mp_exec_mpy_with_exc(buf, len);
    if (exc == MP_OBJ_NULL) {
        // no exception was raised
        return 0;
    } else {
        mp_obj_exception_get_location(exc, exc_loc);
        uint32_t retval = mp_obj_exception_check_system_exit(exc);
        if (retval) {
            // SystemExit was raised, return its argument
            return retval;
        } else {
            // another exception was raised
            return 1;
        }
    }
}

// Execute a Python script passed as pre-compiled bytecode in a buffer.
// If an exception is raised then it's traceback is printed to stdout.
// Returns an exit code:
//  - 0 for normal exit
//  - 1 if a non-SystemExit exception was raised
//  - 128 or'd with return code if a SystemExit exception was raised
uint32_t mp_exec_mpy(const byte *buf, size_t len) {
    mp_obj_t exc = mp_exec_mpy_with_exc(buf, len);
    if (exc == MP_OBJ_NULL) {
        // no exception was raised
        return 0;
    } else {
        uint32_t retval = mp_obj_exception_check_system_exit(exc);
        if (retval) {
            // SystemExit was raised, return its argument
            return retval;
        } else {
            // report all other exceptions by printing them
            mp_obj_print_exception(&mp_plat_print, exc);
            return 1;
        }
    }
}

#if MICROPY_ENABLE_MPY_MODULES

typedef struct _mp_mpy_module_t {
    char *filename;
    mp_raw_code_t *raw_code;
    struct _mp_mpy_module_t *next;
} mp_mpy_module_t;

void mp_mpy_modules_init(void) {
    MP_STATE_PORT(mpy_modules) = NULL;
}

void mp_mpy_modules_register(const byte *buf, size_t len) {
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        mp_mpy_module_t *rf = m_new_obj(mp_mpy_module_t);
        rf->raw_code = mp_raw_code_load_mem(buf, len);
        mp_obj_t f = mp_make_function_from_raw_code(rf->raw_code, MP_OBJ_NULL, MP_OBJ_NULL);
        const char *import_name = qstr_str(mp_obj_fun_get_source(f));
        size_t import_name_len = strlen(import_name) - 3; // cut off .py
        rf->filename = m_new(char, import_name_len + 5);
        memcpy(rf->filename, import_name, import_name_len);
        memcpy(rf->filename + import_name_len, ".mpy", 5);
        rf->next = MP_STATE_PORT(mpy_modules);
        MP_STATE_PORT(mpy_modules) = rf;
        nlr_pop();
    } else {
        // uncaught exception
        mp_obj_t exc = MP_OBJ_FROM_PTR(nlr.ret_val);
        mp_obj_print_exception(&mp_plat_print, exc);
    }
}

#endif

// Hook for the runtime to stat the filesystem for a module.
mp_import_stat_t mp_import_stat(const char *path) {
    #if MICROPY_ENABLE_MPY_MODULES
    for (mp_mpy_module_t *rf = MP_STATE_PORT(mpy_modules); rf != NULL; rf = rf->next) {
        if (strcmp(rf->filename, path) == 0) {
            return MP_IMPORT_STAT_FILE;
        }
    }
    #endif

    // the requested file is not found
    return MP_IMPORT_STAT_NO_EXIST;
}

// Hook for the runtime to load raw code (.mpy) from a file.
mp_raw_code_t *mp_raw_code_load_file(const char *filename) {
    #if MICROPY_ENABLE_MPY_MODULES
    for (mp_mpy_module_t *rf = MP_STATE_PORT(mpy_modules); rf != NULL; rf = rf->next) {
        if (strcmp(rf->filename, filename) == 0) {
            return rf->raw_code;
        }
    }
    #endif

    // the requested file is not found
    nlr_raise(mp_obj_new_exception_msg(&mp_type_RuntimeError, "can't import"));
}

// This is called by the memory manager when a garbage collection is needed.
void gc_collect(void) {
    gc_collect_start();

    #if MICROPY_ENABLE_GC && !MICROPY_ENABLE_IMMORTAL_GC

    extern void gc_helper_get_regs(void *arr);

    // Get all the SPARC registers:
    //   16 regs per window, 32 bit registers, with room to align to 8 bytes
    void *regs[MICROPY_SPARC_NUM_REG_WINDOWS * 16 + 1];
    void **regs_aligned = MP_ALIGN(&regs[0], 8);
    uint32_t state = sparc_disable_interrupts();
    gc_helper_get_regs(regs_aligned);
    sparc_enable_interrupts(state);

    // Scan the stack and the registers (registers live on the stack of this function).
    gc_collect_root(regs_aligned,
        ((uintptr_t)MP_STATE_THREAD(stack_top) - (uintptr_t)regs_aligned) / sizeof(void*));

    #endif

    gc_collect_end();
}

// This is called if there is no exception handler registered to catch an exception.
void nlr_jump_fail(void *val) {
    mp_printf(&mp_plat_print, "fatal error: nlr_jump_fail\n");
    for (;;) {
    }
}

#ifndef NDEBUG
void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr) {
    mp_printf(&mp_plat_print, "Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    for (;;) {
    }
}
#endif
