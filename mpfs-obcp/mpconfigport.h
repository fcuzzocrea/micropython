/*
 * This file is part of the MicroPython port to MPFS platforms
 *
 * Based on LEON port originally made by George Robotics
 *
 * Copyright (c) 2023 Francescodario Cuzzocrea <bosconovic@gmail.com>
 */

// include common options for MPFS
#define MICROPY_RTEMS_ENABLE_VM_MANAGER (1)
#define MICROPY_RTEMS_ENABLE_DATAPOOL (1)
#include "mpfs-common/mpconfigport_common.h"

// options to control how MicroPython is built
#define MICROPY_ALLOC_PATH_MAX      (128)
#define MICROPY_PERSISTENT_CODE_LOAD (1)
#define MICROPY_ENABLE_COMPILER     (0)
#define MICROPY_COMP_CONST          (0)
#define MICROPY_COMP_DOUBLE_TUPLE_ASSIGN (1)
#define MICROPY_COMP_TRIPLE_TUPLE_ASSIGN (1)
#define MICROPY_OPT_LOAD_ATTR_FAST_PATH (1)
#define MICROPY_OPT_MAP_LOOKUP_CACHE (1)
#define MICROPY_ENABLE_GC           (1)
#define MICROPY_ENABLE_IMMORTAL_GC  (0)
#define MICROPY_ENABLE_FINALISER    (1)
#define MICROPY_STACK_CHECK         (1)
#define MICROPY_ENABLE_SOURCE_LINE  (1)
#define MICROPY_ENABLE_DOC_STRING   (0)
#define MICROPY_ERROR_REPORTING     (MICROPY_ERROR_REPORTING_NORMAL)
#define MICROPY_OPT_COMPUTED_GOTO   (1)
#define MICROPY_CPYTHON_COMPAT      (1)
#define MICROPY_LONGINT_IMPL        (MICROPY_LONGINT_IMPL_MPZ)
#define MICROPY_FLOAT_IMPL          (MICROPY_FLOAT_IMPL_DOUBLE)
#define MICROPY_USE_INTERNAL_ERRNO  (1)
#define MICROPY_USE_INTERNAL_PRINTF (0)
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
#define MICROPY_PY_SYS_STDFILES     (0)
#define MICROPY_PY_SYS_STDIO_BUFFER (1)
#define MICROPY_PY_COLLECTIONS_ORDEREDDICT (1)
#define MICROPY_PY_GC               (1)
#define MICROPY_PY_ARRAY            (1)
#define MICROPY_PY_ARRAY_SLICE_ASSIGN (1)
#define MICROPY_PY_ATTRTUPLE        (1)
#define MICROPY_PY_COLLECTIONS      (1)
#define MICROPY_PY_MATH             (1)
#define MICROPY_PY_CMATH            (1)
#define MICROPY_PY_IO               (0)
#define MICROPY_PY_STRUCT           (1)
#define MICROPY_PY_UBINASCII        (1)
#define MICROPY_PY_UCTYPES          (1)
#define MICROPY_PY_UHASHLIB         (1)
#define MICROPY_PY_URANDOM          (1)
#define MICROPY_PY_URANDOM_EXTRA_FUNCS (1)
#define MICROPY_ENABLE_EMERGENCY_EXCEPTION_BUF (1)
#define MICROPY_EMERGENCY_EXCEPTION_BUF_SIZE (256)

// builtin modules
extern const struct _mp_obj_module_t mp_module_time;
extern const struct _mp_obj_module_t mp_module_rtems;
extern const struct _mp_obj_module_t mp_module_mem;
extern const struct _mp_obj_module_t mp_module_datapool;
extern const struct _mp_obj_module_t mp_module_powersys;
#define MICROPY_PORT_BUILTIN_MODULES \
    /* links to core modules to remove the "u" */ \
    { MP_ROM_QSTR(MP_QSTR_struct), MP_ROM_PTR(&mp_module_ustruct) }, \
    { MP_ROM_QSTR(MP_QSTR_binascii), MP_ROM_PTR(&mp_module_ubinascii) }, \
    { MP_ROM_QSTR(MP_QSTR_hashlib), MP_ROM_PTR(&mp_module_uhashlib) }, \
    { MP_ROM_QSTR(MP_QSTR_random), MP_ROM_PTR(&mp_module_urandom) }, \
    \
    /* links to port-specific modules */ \
    { MP_ROM_QSTR(MP_QSTR_time), MP_ROM_PTR(&mp_module_time) }, \
    { MP_ROM_QSTR(MP_QSTR_rtems), MP_ROM_PTR(&mp_module_rtems) }, \
    { MP_ROM_QSTR(MP_QSTR_mem), MP_ROM_PTR(&mp_module_mem) }, \
    { MP_ROM_QSTR(MP_QSTR_datapool), MP_ROM_PTR(&mp_module_datapool) }, \
    { MP_ROM_QSTR(MP_QSTR_powersys), MP_ROM_PTR(&mp_module_powersys) }, \

// Root pointers
#define MICROPY_PORT_ROOT_POINTERS \
    mp_obj_t rtems_script_id; \
    mp_map_t datapool_index; \
    uint32_t rtems_worker_sem_in; \
    uint32_t rtems_worker_sem_out; \
    uint64_t rtems_worker_signal; \
    uint64_t rtems_worker_info0; \
    uint64_t rtems_worker_info1; \

// Hook for the VM
#define MICROPY_VM_HOOK_COUNT (1)
#define MICROPY_VM_HOOK_INIT uint vm_hook_count = MICROPY_VM_HOOK_COUNT;
#define MICROPY_VM_HOOK_LOOP \
    if (--vm_hook_count == 0) { \
        vm_hook_count = MICROPY_VM_HOOK_COUNT; \
        MARK_EXC_IP_SELECTIVE(); \
        mp_vm_hook(code_state); \
    }

struct _mp_code_state_t;
void mp_vm_hook(const struct _mp_code_state_t *code_state);