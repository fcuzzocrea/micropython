/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2015-2016 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Porting of
 * MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
 */

// include common options for LEON/SPARC
#define MICROPY_USE_64BIT_NAN_BOXING (1)
#include "leon-common/mpconfigport_common.h"

// options to control how MicroPython is built
#define MICROPY_NLR_SPARC           (1)
#define MICROPY_ALLOC_PATH_MAX      (128)
#define MICROPY_PERSISTENT_CODE_LOAD (1)
#define MICROPY_ENABLE_COMPILER     (0)
#define MICROPY_COMP_CONST          (0)
#define MICROPY_COMP_DOUBLE_TUPLE_ASSIGN (1)
#define MICROPY_COMP_TRIPLE_TUPLE_ASSIGN (1)
#define MICROPY_ENABLE_GC           (1)
#define MICROPY_ENABLE_IMMORTAL_GC  (1)
#define MICROPY_ENABLE_FINALISER    (0)
#define MICROPY_STACK_CHECK         (1)
#define MICROPY_ENABLE_SOURCE_LINE  (1)
#define MICROPY_ENABLE_DOC_STRING   (0)
#define MICROPY_ERROR_REPORTING     (MICROPY_ERROR_REPORTING_NORMAL)
#define MICROPY_OPT_COMPUTED_GOTO   (1)
#define MICROPY_CPYTHON_COMPAT      (0)
#define MICROPY_LONGINT_IMPL        (MICROPY_LONGINT_IMPL_MPZ)
#define MICROPY_FLOAT_IMPL          (MICROPY_FLOAT_IMPL_DOUBLE)
#define MICROPY_PY_FUNCTION_ATTRS   (1)
#define MICROPY_PY_BUILTINS_STR_UNICODE (1)
#define MICROPY_PY_BUILTINS_STR_SPLITLINES (1)
#define MICROPY_PY_BUILTINS_MEMORYVIEW (1)
#define MICROPY_PY_BUILTINS_FROZENSET (1)
#define MICROPY_PY_MICROPYTHON_MEM_INFO (1)
#define MICROPY_PY_SYS              (1)
#define MICROPY_PY_SYS_EXIT         (1)
#define MICROPY_PY_SYS_MAXSIZE      (1)
#define MICROPY_PY_SYS_MODULES      (0)
#define MICROPY_PY_SYS_PLATFORM     "leon2"
#define MICROPY_PY_COLLECTIONS_ORDEREDDICT (1)
#define MICROPY_PY_GC               (1)
#define MICROPY_PY_ARRAY            (1)
#define MICROPY_PY_ARRAY_SLICE_ASSIGN (1)
#define MICROPY_PY_ATTRTUPLE        (1)
#define MICROPY_PY_COLLECTIONS      (1)
#define MICROPY_PY_MATH             (1)
#define MICROPY_PY_CMATH            (0)
#define MICROPY_PY_UBINASCII        (0)
#define MICROPY_PY_IO               (0)
#define MICROPY_PY_STRUCT           (1)
#define MICROPY_ENABLE_EMERGENCY_EXCEPTION_BUF (1)
#define MICROPY_EMERGENCY_EXCEPTION_BUF_SIZE (256)
#define MICROPY_ENABLE_MPY_MODULES  (1)

// builtin modules
extern const struct _mp_obj_module_t mp_module_time;
extern const struct _mp_obj_module_t mp_module_rtems;
extern const struct _mp_obj_module_t mp_module_mem;
#define MICROPY_PORT_BUILTIN_MODULES \
    { MP_ROM_QSTR(MP_QSTR_time), MP_ROM_PTR(&mp_module_time) }, \
    { MP_ROM_QSTR(MP_QSTR_rtems), MP_ROM_PTR(&mp_module_rtems) }, \
    { MP_ROM_QSTR(MP_QSTR_mem), MP_ROM_PTR(&mp_module_mem) }, \

// Root pointers
#define MICROPY_PORT_ROOT_POINTERS \
    mp_obj_t rtems_script_id; \
    struct _mp_mpy_module_t *mpy_modules; \

