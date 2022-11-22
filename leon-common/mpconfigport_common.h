/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2015-2017 George Robotics Limited
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

// definitions specific to SPARC
#define MP_ENDIANNESS_BIG (1)
#define MICROPY_MAKE_POINTER_CALLABLE(p) ((void*)p)
#define MICROPY_SPARC_NUM_REG_WINDOWS (8)

#if defined(MICROPY_USE_64BIT_NAN_BOXING) && MICROPY_USE_64BIT_NAN_BOXING

// configuration for 64-bit NaN boxing
#define MICROPY_OBJ_REPR (MICROPY_OBJ_REPR_D)
#define UINT_FMT "%llu"
#define INT_FMT "%lld"
typedef int64_t mp_int_t;
typedef uint64_t mp_uint_t;

#else

// configuration for object size the same as machine pointer size
#define MICROPY_OBJ_REPR (MICROPY_OBJ_REPR_A)
typedef intptr_t mp_int_t;
typedef uintptr_t mp_uint_t;

#endif

typedef long mp_off_t;

#if RTEMS_4_8_EDISOFT

// Edisoft doesn't have alloca(), so use m_malloc
#define MICROPY_NO_ALLOCA (1)

// Edisoft doesn't define offsetof, so use the RTEMS version
#define offsetof(x, y) RTEMS_offsetof(x, y)

#else

// We need to provide a declaration/definition of alloca()
#include <alloca.h>

#endif

// INFINITY is not defined by toolchain
#ifndef INFINITY
#define INFINITY (__builtin_inff())
#endif

// LEON ports provide their own version of mp_raw_code_load_file
#define MICROPY_CUSTOM_MP_RAW_CODE_LOAD_FILE (1)

#if RTEMS_4

// For RTEMS 4.x use a custom NLR handler (custom setjmp/longjmp).
// This is strictly only needed for RTEMS_4_8_EDISOFT, but it also works on other RTEMS 4.x versions.
#if !defined(MICROPY_NLR_SPARC) && !defined(MICROPY_NLR_SETJMP)
#define MICROPY_NLR_SPARC (1)
#endif

// For RTEMS 4.x these thread-local-state functions can be inline
#include <rtems.h>
static inline void *mp_state_ptr(void) {
    return ((void *)_Thread_Executing->Start.numeric_argument);
}
static inline void mp_state_ptr_set(void *value) {
    _Thread_Executing->Start.numeric_argument = (uintptr_t)value;
}

#else

// For RTEMS 5 and RTEMS 6 it's possible to use the built-in setjmp/longjmp functions
// for MicroPython NLR handling.
#if !defined(MICROPY_NLR_SPARC) && !defined(MICROPY_NLR_SETJMP)
#define MICROPY_NLR_SETJMP (1)
#endif

// For RTEMS 5 and RTEMS 6 these thread-local-state functions are defined as real functions
void *mp_state_ptr(void);
void mp_state_ptr_set(void *value);
#endif

// We define our own state accessor macros, to use a thread-local-state pointer in the RTEMS task
#define MP_STATE_PTR ((mp_state_ctx_t*)mp_state_ptr())
#define MP_STATE_VM(x) (MP_STATE_PTR->vm.x)
#define MP_STATE_MEM(x) (MP_STATE_PTR->mem.x)
#define MP_STATE_THREAD(x) (MP_STATE_PTR->thread.x)
#define MP_STATE_PORT MP_STATE_VM

// The static analyser doesn't understand __builtin_expect so don't use it
#define MP_LIKELY(x) (x)
#define MP_UNLIKELY(x) (x)

// Enable math function workarounds for cases that do not conform to IEEE floating point behaviour.

#if RTEMS_4_8
#define MICROPY_PY_MATH_MODF_CHECK_DOMAIN (1)
#define MICROPY_PY_MATH_ASIN_CHECK_DOMAIN (1)
#define MICROPY_PY_MATH_ACOS_CHECK_DOMAIN (1)
#endif

#if RTEMS_4_10 || RTEMS_4_11
#define MICROPY_PY_BUILTINS_FLOAT_POW_CHECK_DOMAIN (1)
#define MICROPY_PY_MATH_MODF_CHECK_DOMAIN (1)
#endif

#if RTEMS_5
#define MICROPY_PY_BUILTINS_FLOAT_POW_CHECK_DOMAIN (1)
#define MICROPY_PY_MATH_MODF_CHECK_DOMAIN (1)
#endif

#if RTEMS_6
#define MICROPY_PY_BUILTINS_ROUND_POW_CHECK_DOMAIN (1)
#define MICROPY_PY_MATH_SQRT_CHECK_DOMAIN (1)
#define MICROPY_PY_MATH_POW_CHECK_DOMAIN (1)
#define MICROPY_PY_MATH_FMOD_CHECK_DOMAIN (1)
#define MICROPY_PY_MATH_ASIN_CHECK_DOMAIN (1)
#define MICROPY_PY_MATH_ACOS_CHECK_DOMAIN (1)
#endif
