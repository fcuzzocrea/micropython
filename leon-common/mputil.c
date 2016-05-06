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
#include "py/mphal.h"
#include "leon-common/mputil.h"
#include "leon-common/mpvmmanage.h"

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
    //qstr simple_name = bc[0] | (bc[1] << 8);
    qstr source_file = bc[2] | (bc[3] << 8);

    return source_file;
}

// Get the line number info of some currently-executing bytecode.
size_t mp_code_state_get_line(const mp_code_state *code_state, qstr *source_file, qstr *block_name) {
    const byte *ip = code_state->code_info;
    mp_uint_t code_info_size = mp_decode_uint(&ip);
    #if MICROPY_PERSISTENT_CODE
    *block_name = ip[0] | (ip[1] << 8);
    *source_file = ip[2] | (ip[3] << 8);
    ip += 4;
    #else
    *block_name = mp_decode_uint(&ip);
    *source_file = mp_decode_uint(&ip);
    #endif
    size_t bc = code_state->ip - code_state->code_info - code_info_size;
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

#if MICROPY_ENABLE_COMPILER
// Execute a Python script passed a a string.
void mp_exec_str(const char *src, mp_parse_input_kind_t input_kind) {
    mp_vm_manager_init(RTEMS_SELF);

    mp_lexer_t *lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, src, strlen(src), 0);
    if (lex == NULL) {
        printf("MemoryError: lexer could not allocate memory\n");
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

// Execute a Python script passed as pre-compiled bytecode in a buffer.
// Returns an exit code, 0 for normal exit.
uint32_t mp_exec_mpy(const byte *buf, size_t len) {
    mp_vm_worker_init();
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        mp_store_global(MP_QSTR___file__, MP_OBJ_NEW_QSTR(MP_QSTR___main__));
        mp_raw_code_t *raw_code = mp_raw_code_load_mem(buf, len);
        mp_obj_t f = mp_make_function_from_raw_code(raw_code, MP_OBJ_NULL, MP_OBJ_NULL);
        mp_call_function_0(f);
        nlr_pop();
        return 0;
    } else {
        // uncaught exception
        mp_obj_t exc = MP_OBJ_FROM_PTR(nlr.ret_val);

        // check for SystemExit
        if (mp_obj_is_subclass_fast(MP_OBJ_FROM_PTR(mp_obj_get_type(exc)),
            MP_OBJ_FROM_PTR(&mp_type_SystemExit))) {
            // None is an exit value of 0; an int is its value; anything else is 1
            mp_obj_t exit_val = mp_obj_exception_get_value(exc);
            mp_int_t val = 0;
            if (exit_val != mp_const_none && !mp_obj_get_int_maybe(exit_val, &val)) {
                val = 1;
            }
            return 0x80 | (val & 255);
        }

        // report all other exceptions
        mp_obj_print_exception(&mp_plat_print, exc);
        return 1;
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
    gc_helper_get_regs(MP_ALIGN(regs, 8));

    // Scan the stack and the registers (registers live on the stack of this function).
    void **regs_ptr = &regs[0];
    gc_collect_root(regs_ptr,
        ((uintptr_t)MP_STATE_VM(stack_top) - (uintptr_t)regs_ptr) / sizeof(void*));

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
