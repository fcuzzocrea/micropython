/*
 * This file is part of the MicroPython port to MPFS platforms
 *
 * Based on LEON port originally made by George Robotics
 *
 * Copyright (c) 2023 Francescodario Cuzzocrea <bosconovic@gmail.com>
 */

#include <stdint.h>

// Whether the VM manager helper functions are enabled
#ifndef MICROPY_RTEMS_ENABLE_VM_MANAGER
#define MICROPY_RTEMS_ENABLE_VM_MANAGER (0)
#endif

// Whether the datapool module is enabled
#ifndef MICROPY_RTEMS_ENABLE_DATAPOOL
#define MICROPY_RTEMS_ENABLE_DATAPOOL (0)
#endif

#define MICROPY_MAKE_POINTER_CALLABLE(p) ((void *)((mp_uint_t)(p) | 1))

// configuration for 64-bit NaN boxing
#define UINT_FMT "%u"
#define INT_FMT "%d"

// Configuration for LP64D
typedef long mp_int_t; // must be pointer size
typedef unsigned long mp_uint_t; // must be pointer size
typedef long long mp_off_t;

// We need to provide a declaration/definition of alloca()
#include <alloca.h>

// MPFS ports provide their own version of mp_raw_code_load_file
#define MICROPY_CUSTOM_MP_RAW_CODE_LOAD_FILE (1)

// For RTEMS 6 it's possible to use the built-in setjmp/longjmp functions for MicroPython NLR handling.
#define MICROPY_NLR_SETJMP (1)

// For RTEMS 6 these thread-local-state functions are defined as real functions
void *mp_state_ptr(void);
void mp_state_ptr_set(void *value);

// We define our own state accessor macros, to use a thread-local-state pointer in the RTEMS task
#define MP_STATE_PTR ((mp_state_ctx_t*)mp_state_ptr())
#define MP_STATE_VM(x) (MP_STATE_PTR->vm.x)
#define MP_STATE_MEM(x) (MP_STATE_PTR->mem.x)
#define MP_STATE_THREAD(x) (MP_STATE_PTR->thread.x)
#define MP_STATE_PORT MP_STATE_VM

// Enable math function workarounds for cases that do not conform to IEEE floating point behaviour.
#define MICROPY_PY_BUILTINS_ROUND_POW_CHECK_DOMAIN (1)
#define MICROPY_PY_MATH_SQRT_CHECK_DOMAIN (1)
#define MICROPY_PY_MATH_POW_CHECK_DOMAIN (1)
#define MICROPY_PY_MATH_FMOD_CHECK_DOMAIN (1)
#define MICROPY_PY_MATH_ASIN_CHECK_DOMAIN (1)
#define MICROPY_PY_MATH_ACOS_CHECK_DOMAIN (1)

// Set the sys.platform value based on the LEON target.
#define MICROPY_PY_SYS_PLATFORM "mpfs"
