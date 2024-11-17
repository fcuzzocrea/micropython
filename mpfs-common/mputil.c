#include <string.h>

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
#include "mpfs-common/mputil.h"

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
        // The check for values!=NULL is to silence Polyspace
        if (n >= 3 && values != NULL) {
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
        #if MICROPY_PY___FILE__
        mp_store_global(MP_QSTR___file__, MP_OBJ_NEW_QSTR(MP_QSTR___main__));
        #endif
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
    #else
    (void)path;
    #endif

    // the requested file is not found
    return MP_IMPORT_STAT_NO_EXIST;
}

#if MICROPY_PY_IO
// Hook for the runtime to open a file.
mp_obj_t mp_builtin_open(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
    nlr_raise(mp_obj_new_exception_msg(&mp_type_NotImplementedError, "open not implemented"));
}
MP_DEFINE_CONST_FUN_OBJ_KW(mp_builtin_open_obj, 1, mp_builtin_open);
#endif

// Hook for the runtime to load raw code (.mpy) from a file.
mp_raw_code_t *mp_raw_code_load_file(const char *filename) {
    #if MICROPY_ENABLE_MPY_MODULES
    for (mp_mpy_module_t *rf = MP_STATE_PORT(mpy_modules); rf != NULL; rf = rf->next) {
        if (strcmp(rf->filename, filename) == 0) {
            return rf->raw_code;
        }
    }
    #else
    (void)filename;
    #endif

    // the requested file is not found
    nlr_raise(mp_obj_new_exception_msg(&mp_type_RuntimeError, "can't import"));
}

#include <rtems.h> // for riscv_interrupt_disable, riscv_interrupt_enable

void gc_helper_get_regs(void *arr);

// This is called by the memory manager when a garbage collection is needed.
void gc_collect(void) {
    gc_collect_start();

    #if MICROPY_ENABLE_GC && !MICROPY_ENABLE_IMMORTAL_GC

    // Get all all general purpose registers and SP:
    uint32_t level = riscv_interrupt_disable();

    register void *reg0 asm ("x0");
    register void *reg1 asm ("x1");
    register void *reg2 asm ("x2");
    register void *reg3 asm ("x3");
    register void *reg4 asm ("x4");
    register void *reg5 asm ("x5");
    register void *reg6 asm ("x6");
    register void *reg7 asm ("x7");
    register void *reg8 asm ("x8");
    register void *reg9 asm ("x9");
    register void *reg10 asm ("x10");
    register void *reg11 asm ("x11");
    register void *reg12 asm ("x12");
    register void *reg13 asm ("x13");
    register void *reg14 asm ("x14");
    register void *reg15 asm ("x15");
    register void *reg16 asm ("x16");
    register void *reg17 asm ("x17");
    register void *reg18 asm ("x18");
    register void *reg19 asm ("x19");
    register void *reg20 asm ("x20");
    register void *reg21 asm ("x21");
    register void *reg22 asm ("x22");
    register void *reg23 asm ("x23");
    register void *reg24 asm ("x24");
    register void *reg25 asm ("x25");
    register void *reg26 asm ("x26");
    register void *reg27 asm ("x27");
    register void *reg28 asm ("x28");
    register void *reg29 asm ("x29");
    register void *reg30 asm ("x30");
    register void *reg31 asm ("x31");
    register void *regSP asm ("sp");

    // dump registers into an array, so they will be on the stack and will therefore
    // be found when running the gc collect
    void *regs[33] = { reg0, reg1, reg2, reg3, reg4, reg5,
                       reg6, reg7, reg8, reg9, reg10, reg11,
                       reg12, reg13, reg14, reg15, reg16,
                       reg17, reg18, reg19, reg20, reg21,
                       reg22, reg23, reg24, reg25, reg26,
                       reg27, reg28, reg29, reg30, reg31, regSP };
    void **regs_ptr = (void **)&regs;

    riscv_interrupt_enable(level);

    // Scan the stack and the registers (registers live on the stack of this function).
    gc_collect_root(regs_ptr,
        ((uintptr_t)MP_STATE_THREAD(stack_top) - (uintptr_t)regs_ptr) / sizeof(void*));

    #endif

    gc_collect_end();
}

// This is called if there is no exception handler registered to catch an exception.
void nlr_jump_fail(void *val) {
    mp_printf(&mp_plat_print, "FATAL: uncaught NLR %p\n", val);
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
