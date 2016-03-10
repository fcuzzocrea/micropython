/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2015-2016 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Porting of
 * MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
 */

// options to control how MicroPython is built
#define MICROPY_NLR_SETJMP          (1)
#define MICROPY_ALLOC_PATH_MAX      (128)
#define MICROPY_PERSISTENT_CODE_LOAD (1)
#define MICROPY_EMIT_X64            (0)
#define MICROPY_EMIT_THUMB          (0)
#define MICROPY_EMIT_INLINE_THUMB   (0)
#define MICROPY_ENABLE_COMPILER     (0)
#define MICROPY_COMP_MODULE_CONST   (0)
#define MICROPY_COMP_CONST          (0)
#define MICROPY_COMP_DOUBLE_TUPLE_ASSIGN (1)
#define MICROPY_COMP_TRIPLE_TUPLE_ASSIGN (1)
#define MICROPY_MEM_STATS           (0)
#define MICROPY_DEBUG_PRINTERS      (0)
#define MICROPY_ENABLE_GC           (1)
#define MICROPY_ENABLE_IMMORTAL_GC  (0)
#define MICROPY_ENABLE_FINALISER    (1)
#define MICROPY_STACK_CHECK         (1)
#define MICROPY_REPL_EVENT_DRIVEN   (0)
#define MICROPY_HELPER_REPL         (1)
#define MICROPY_HELPER_LEXER_UNIX   (0)
#define MICROPY_ENABLE_SOURCE_LINE  (1)
#define MICROPY_ENABLE_DOC_STRING   (0)
#define MICROPY_ERROR_REPORTING     (MICROPY_ERROR_REPORTING_NORMAL)
#define MICROPY_OPT_COMPUTED_GOTO   (1)
#define MICROPY_OPT_CACHE_MAP_LOOKUP_IN_BYTECODE (0)
#define MICROPY_MODULE_WEAK_LINKS   (1)
#define MICROPY_MODULE_FROZEN       (0)
#define MICROPY_CPYTHON_COMPAT      (1)
#define MICROPY_LONGINT_IMPL        (MICROPY_LONGINT_IMPL_MPZ)
#define MICROPY_FLOAT_IMPL          (MICROPY_FLOAT_IMPL_DOUBLE)
#define MICROPY_PY_FUNCTION_ATTRS   (1)
#define MICROPY_PY_BUILTINS_STR_UNICODE (1)
#define MICROPY_PY_BUILTINS_STR_SPLITLINES (1)
#define MICROPY_PY_BUILTINS_BYTEARRAY (1)
#define MICROPY_PY_BUILTINS_MEMORYVIEW (1)
#define MICROPY_PY_BUILTINS_ENUMERATE (1)
#define MICROPY_PY_BUILTINS_FROZENSET (1)
#define MICROPY_PY_BUILTINS_REVERSED (1)
#define MICROPY_PY_BUILTINS_SET     (1)
#define MICROPY_PY_BUILTINS_SLICE   (1)
#define MICROPY_PY_BUILTINS_PROPERTY (1)
#define MICROPY_PY___FILE__         (1)
#define MICROPY_PY_MICROPYTHON_MEM_INFO (1)
#define MICROPY_PY_SYS_EXIT         (1)
#define MICROPY_PY_SYS_MAXSIZE      (1)
#define MICROPY_PY_SYS_MODULES      (0)
#define MICROPY_PY_SYS_STDFILES     (0)
#define MICROPY_PY_SYS_STDIO_BUFFER (1)
#define MICROPY_PY_SYS_PLATFORM     "leon2"
#define MICROPY_PY_COLLECTIONS_ORDEREDDICT (1)
#define MICROPY_PY_GC               (1)
#define MICROPY_PY_ARRAY            (1)
#define MICROPY_PY_ARRAY_SLICE_ASSIGN (1)
#define MICROPY_PY_ATTRTUPLE        (1)
#define MICROPY_PY_COLLECTIONS      (1)
#define MICROPY_PY_MATH             (1)
#define MICROPY_PY_CMATH            (1)
#define MICROPY_PY_UJSON            (1)
#define MICROPY_PY_UBINASCII        (1)
#define MICROPY_PY_IO               (0)
#define MICROPY_PY_STRUCT           (1)
#define MICROPY_PY_SYS              (1)
#define MICROPY_ENABLE_EMERGENCY_EXCEPTION_BUF (1)
#define MICROPY_EMERGENCY_EXCEPTION_BUF_SIZE (256)

// builtin modules
extern const struct _mp_obj_module_t mp_module_time;
extern const struct _mp_obj_module_t mp_module_rtems;
#define MICROPY_PORT_BUILTIN_MODULES \
    { MP_ROM_QSTR(MP_QSTR_time), MP_ROM_PTR(&mp_module_time) }, \
    { MP_ROM_QSTR(MP_QSTR_rtems), MP_ROM_PTR(&mp_module_rtems) }, \

// definitions specific to SPARC
#define MP_ENDIANNESS_BIG (1)
#define MICROPY_MAKE_POINTER_CALLABLE(p) ((void*)p)
#define MICROPY_SPARC_NUM_REG_WINDOWS (8)

#include <stdint.h>

#if 0

// configuration for 32-bit object word size
#define MICROPY_OBJ_REPR (MICROPY_OBJ_REPR_A)
#define BYTES_PER_WORD (4)
#define UINT_FMT "%u"
#define INT_FMT "%d"
typedef int mp_int_t;
typedef unsigned mp_uint_t;

#else

// configuration for 64-bit NaN boxing
#define MICROPY_OBJ_REPR (MICROPY_OBJ_REPR_D)
#define BYTES_PER_WORD (8)
typedef int64_t mp_int_t;
typedef uint64_t mp_uint_t;
#define UINT_FMT "%llu"
#define INT_FMT "%lld"

#endif

typedef void *machine_ptr_t; // must be of pointer size
typedef const void *machine_const_ptr_t; // must be of pointer size
typedef long mp_off_t;

// We need to provide a declaration/definition of alloca()
#include <alloca.h>

// INFINITY is not defined by toolchain
#ifndef INFINITY
#define INFINITY (__builtin_inff())
#endif

// We define our own state accessor macros
#include <rtems.h>
#define MP_STATE_PTR ((mp_state_ctx_t*)_Thread_Executing->Start.numeric_argument)
#define MP_STATE_CTX(x) (MP_STATE_PTR->x)
#define MP_STATE_VM(x) (MP_STATE_PTR->vm.x)
#define MP_STATE_MEM(x) (MP_STATE_PTR->mem.x)
#define MP_STATE_PORT MP_STATE_VM

// Hook for the VM
#define MICROPY_VM_HOOK_COUNT (1)
#define MICROPY_VM_HOOK_INIT uint vm_hook_count = MICROPY_VM_HOOK_COUNT;
#define MICROPY_VM_HOOK_LOOP \
    if (--vm_hook_count == 0) { \
        vm_hook_count = MICROPY_VM_HOOK_COUNT; \
        MARK_EXC_IP_SELECTIVE(); \
        extern void mp_vm_hook(const mp_code_state *code_state); \
        mp_vm_hook(code_state); \
    }
