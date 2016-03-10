/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2015-2016 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Porting of
 * MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
 */

// options to control how MicroPython is built

#define MICROPY_ALLOC_PATH_MAX      (PATH_MAX)
#define MICROPY_PERSISTENT_CODE_LOAD (0)
#define MICROPY_PERSISTENT_CODE_SAVE (1)

#define MICROPY_EMIT_X64            (0)
#define MICROPY_EMIT_X86            (0)
#define MICROPY_EMIT_THUMB          (0)
#define MICROPY_EMIT_INLINE_THUMB   (0)
#define MICROPY_EMIT_INLINE_THUMB_ARMV7M (0)
#define MICROPY_EMIT_INLINE_THUMB_FLOAT (0)
#define MICROPY_EMIT_ARM            (0)

#define MICROPY_COMP_CONST_FOLDING  (1)
#define MICROPY_COMP_MODULE_CONST   (1)
#define MICROPY_COMP_CONST          (1)
#define MICROPY_COMP_DOUBLE_TUPLE_ASSIGN (1)
#define MICROPY_COMP_TRIPLE_TUPLE_ASSIGN (1)

#define MICROPY_OPT_CACHE_MAP_LOOKUP_IN_BYTECODE (0)

#define MICROPY_ENABLE_RUNTIME      (0)
#define MICROPY_ENABLE_GC           (1)
#define MICROPY_STACK_CHECK         (1)
#define MICROPY_HELPER_LEXER_UNIX   (1)
#define MICROPY_LONGINT_IMPL        (MICROPY_LONGINT_IMPL_MPZ)
#define MICROPY_ENABLE_SOURCE_LINE  (1)
#define MICROPY_ENABLE_DOC_STRING   (0)
#define MICROPY_ERROR_REPORTING     (MICROPY_ERROR_REPORTING_DETAILED)
#define MICROPY_WARNINGS            (1)

#define MICROPY_FLOAT_IMPL          (MICROPY_FLOAT_IMPL_DOUBLE)
#define MICROPY_CPYTHON_COMPAT      (1)

#define MICROPY_PY_BUILTINS_STR_UNICODE (1)

// Define to 1 to use inefficient GC helper implementation
// (if more efficient arch-specific one is not available).
#define MICROPY_GCREGS_SETJMP       (0)

#define MICROPY_PY___FILE__         (0)
#define MICROPY_PY_ARRAY            (0)
#define MICROPY_PY_ATTRTUPLE        (0)
#define MICROPY_PY_COLLECTIONS      (0)
#define MICROPY_PY_MATH             (0)
#define MICROPY_PY_CMATH            (0)
#define MICROPY_PY_GC               (0)
#define MICROPY_PY_IO               (0)
#define MICROPY_PY_SYS              (0)

// type definitions for the specific machine

#ifdef __LP64__
typedef long mp_int_t; // must be pointer size
typedef unsigned long mp_uint_t; // must be pointer size
#else
// These are definitions for machines where sizeof(int) == sizeof(void*),
// regardless for actual size.
typedef int mp_int_t; // must be pointer size
typedef unsigned int mp_uint_t; // must be pointer size
#endif

#define BYTES_PER_WORD sizeof(mp_int_t)

// Need to override the size of a small int so that it's compatible with
// the LEON system.
#define MP_SMALL_INT_MIN (-((mp_int_t)0x40000000))
#define MP_SMALL_INT_FITS(n) ((((n) ^ ((n) << 1)) & ~((mp_uint_t)0x7fffffff)) == 0)
#define MP_SMALL_INT_POSITIVE_MASK ~(MP_SMALL_INT_MIN)

// Cannot include <sys/types.h>, as it may lead to symbol name clashes
#if _FILE_OFFSET_BITS == 64 && !defined(__LP64__)
typedef long long mp_off_t;
#else
typedef long mp_off_t;
#endif

typedef void *machine_ptr_t; // must be of pointer size
typedef const void *machine_const_ptr_t; // must be of pointer size

#define MP_PLAT_PRINT_STRN(str, len) (void)0

// We need to provide a declaration/definition of alloca()
#ifdef __FreeBSD__
#include <stdlib.h>
#else
#include <alloca.h>
#endif

#include <stdint.h>
