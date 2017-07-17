/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2015-2017 George Robotics Limited
 */

#include <stdint.h>

// definitions specific to SPARC
#define MP_ENDIANNESS_BIG (1)
#define MICROPY_MAKE_POINTER_CALLABLE(p) ((void*)p)
#define MICROPY_SPARC_NUM_REG_WINDOWS (8)

#if defined(MICROPY_USE_64BIT_NAN_BOXING) && MICROPY_USE_64BIT_NAN_BOXING

// configuration for 64-bit NaN boxing
#define MICROPY_OBJ_REPR (MICROPY_OBJ_REPR_D)
#define BYTES_PER_WORD (8)
typedef int64_t mp_int_t;
typedef uint64_t mp_uint_t;
#define UINT_FMT "%llu"
#define INT_FMT "%lld"

#else

// configuration for 32-bit object word size
#define MICROPY_OBJ_REPR (MICROPY_OBJ_REPR_A)
#define BYTES_PER_WORD (4)
#define UINT_FMT "%u"
#define INT_FMT "%d"
typedef int mp_int_t;
typedef unsigned mp_uint_t;

#endif

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
#define MP_STATE_VM(x) (MP_STATE_PTR->vm.x)
#define MP_STATE_MEM(x) (MP_STATE_PTR->mem.x)
#define MP_STATE_THREAD(x) (MP_STATE_PTR->thread.x)
#define MP_STATE_PORT MP_STATE_VM
