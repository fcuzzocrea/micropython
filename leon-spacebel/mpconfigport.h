/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2015 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

// include common options for LEON/SPARC
#define MICROPY_USE_64BIT_NAN_BOXING (1)
#include "leon-common/mpconfigport_common.h"

#define MICROPY_ENABLE_PYSTACK (1)
#define MICROPY_STACKLESS (1)
#define MICROPY_STACKLESS_STRICT (1)
#define MICROPY_ENABLE_SCHEDULER (0)
#define MICROPY_FLOAT_HIGH_QUALITY_HASH  (0)
#define MICROPY_MODULE_FROZEN_MPY (0)
#define MICROPY_MODULE_FROZEN_STR (0)
#define MICROPY_FULL_CHECK (1)
#define MICROPY_KBD_EXCEPTION (0)
#define MICROPY_PY_BUILTINS_INPUT (0)
#define MICROPY_PY_FRAMEBUF (0)
#define MICROPY_PY_IO_BUFFEREDWRITER (0)
#define MICROPY_PY_IO_RESOURCE_STREAM (0)
#define MICROPY_PY_MACHINE_I2C (0)
#define MICROPY_PY_MACHINE_PULSE (0)
#define MICROPY_PY_MACHINE_SPI (0)
#define MICROPY_PY_THREAD (0)
#define MICROPY_PY_THREAD_GIL (0)
#define MICROPY_PY_THREAD_GIL_DIVISOR (0)
#define MICROPY_PY_USELECT (0)
#define MICROPY_PY_UTIMEQ (0)
#define MICROPY_PY_UTIME_MP_HAL (0)
#define MICROPY_PY_WEBSOCKET (0)
#define MICROPY_READER_VFS (0)
#define MICROPY_SCHEDULER_DEPTH (0)
#define MICROPY_STREAMS_POSIX_API (0)
#define MICROPY_ALLOC_GC_STACK_SIZE (64)
#define MICROPY_GC_CONSERVATIVE_CLEAR (MICROPY_ENABLE_GC)
#define MICROPY_GC_ALLOC_THRESHOLD (1)
#define MICROPY_ALLOC_QSTR_CHUNK_INIT (128)
#define MICROPY_ALLOC_LEXER_INDENT_INIT (10)
#define MICROPY_ALLOC_LEXEL_INDENT_INC (8)
#define MICROPY_ALLOC_PARSE_RULE_INIT (64)
#define MICROPY_ALLOC_PARSE_RULE_INC (16)
#define MICROPY_ALLOC_PARSE_RESULT_INIT (32)
#define MICROPY_ALLOC_PARSE_RESULT_INC (16)
#define MICROPY_ALLOC_PARSE_INTERN_STRING_LEN (10)
#define MICROPY_ALLOC_PARSE_CHUNK_INIT (128)
#define MICROPY_ALLOC_SCOPE_ID_INIT (4)
#define MICROPY_ALLOC_SCOPE_ID_INC (6)
#define MICROPY_ALLOC_PATH_MAX (512)
#define MICROPY_MODULE_DICT_SIZE (1)
#define MICROPY_MALLOC_USES_ALLOCATED_SIZE (0)
#define MICROPY_QSTR_BYTES_IN_LEN (1)
#define MICROPY_QSTR_BYTES_IN_HASH (2)
#define MICROPY_EMIT_X64 (0)
#define MICROPY_EMIT_X86 (0)
#define MICROPY_EMIT_THUMB (0)
#define MICROPY_EMIT_INLINE_THUMB (0)
#define MICROPY_EMIT_INLINE_THUMB_ARMV7M (1)
#define MICROPY_EMIT_INLINE_THUMB_FLOAT (1)
#define MICROPY_EMIT_ARM (0)
#define MICROPY_EMIT_XTENSA (0)
#define MICROPY_EMIT_INLINE_XTENSA (0)
#define MICROPY_COMP_CONST_FOLDING (1)
#define MICROPY_COMP_MODULE_CONST (0)
#define MICROPY_COMP_CONST (1)
#define MICROPY_COMP_DOUBLE_TUPLE_ASSIGN (1)
#define MICROPY_COMP_TRIPLE_TUPLE_ASSIGN (0)
#define MICROPY_COMP_RETURN_IF_EXPR (0)
#define MICROPY_MEM_STATS (0)
#define MICROPY_DEBUG_PRINTERS (0)
#define MICROPY_OPT_COMPUTED_GOTO (0)
#define MICROPY_OPT_CACHE_MAP_LOOKUP_IN_BYTECODE (0)
#define MICROPY_OPT_MPZ_BITWISE (0)
#define MICROPY_READER_VFS (0)
#define MICROPY_ENABLE_GC (1)
#define MICROPY_ENABLE_IMMORTAL_GC (0)
#define MICROPY_ENABLE_FINALISER (0)
#define MICROPY_STACK_CHECK (1)
#define MICROPY_ENABLE_EMERGENCY_EXCEPTION_BUF (1)
#define MICROPY_EMERGENCY_EXCEPTION_BUF_SIZE (256)
#define MICROPY_KBD_EXCEPTION (0)
#define MICROPY_ASYNC_KBD_INTR (0)
#define MICROPY_HELPER_REPL (0)
#define MICROPY_REPL_EMACS_KEYS (0)
#define MICROPY_REPL_AUTO_INDENT (0)
#define MICROPY_REPL_EVENT_DRIVEN (0)
#define MICROPY_HELPER_LEXER_UNIX (0)
#define MICROPY_LONGINT_IMPL (MICROPY_LONGINT_IMPL_MPZ)
#define MICROPY_ENABLE_SOURCE_LINE (1)
#define MICROPY_ENABLE_DOC_STRING (0)
#define MICROPY_ERROR_REPORTING (MICROPY_ERROR_REPORTING_NORMAL)
#define MICROPY_FLOAT_IMPL (MICROPY_FLOAT_IMPL_DOUBLE)
#define MICROPY_PY_BUILTINS_COMPLEX (0)
#define MICROPY_CPYTHON_COMPAT (1)
#define MICROPY_PY_ASYNC_AWAIT (0)
#define MICROPY_PY_BUILTINS_STR_UNICODE (0)
#define MICROPY_PY_BUILTINS_STR_CENTER (0)
#define MICROPY_PY_BUILTINS_STR_PARTITION (0)
#define MICROPY_PY_BUILTINS_STR_SPLITLINES (1)
#define MICROPY_PY_BUILTINS_BYTEARRAY (1)
#define MICROPY_PY_BUILTINS_MEMORYVIEW (1)
#define MICROPY_PY_BUILTINS_SET (1)
#define MICROPY_PY_BUILTINS_SLICE (1)
#define MICROPY_PY_BUILTINS_SLICE_ATTRS (0)
#define MICROPY_PY_BUILTINS_FROZENSET (0)
#define MICROPY_PY_BUILTINS_PROPERTY (0)
#define MICROPY_PY_BUILTINS_RANGE_ATTRS (1)
#define MICROPY_PY_BUILTINS_TIMEOUTERROR (0)
#define MICROPY_PY_ALL_SPECIAL_METHODS (0)
#define MICROPY_PY_BUILTINS_COMPILE (0)
#define MICROPY_PY_BUILTINS_ENUMERATE (1)
#define MICROPY_PY_BUILTINS_EXECFILE (0)
#define MICROPY_PY_BUILTINS_FILTER (1)
#define MICROPY_PY_BUILTINS_REVERSED (1)
#define MICROPY_PY_BUILTINS_NOTIMPLEMENTED (0)
#define MICROPY_PY_BUILTINS_MIN_MAX (1)
#define MICROPY_PY_BUILTINS_POW3 (0)
#define MICROPY_PY_BUILTINS_HELP (0)
#define MICROPY_PY_BUILTINS_HELP_MODULES (0)
#define MICROPY_PY_ARRAY (1)
#define MICROPY_PY_ARRAY_SLICE_ASSIGN (1)
#define MICROPY_PY_ATTRTUPLE (0)
#define MICROPY_PY_COLLECTIONS (1)
#define MICROPY_PY_COLLECTIONS_ORDEREDDICT (0)
#define MICROPY_PY_MATH (1)
#define MICROPY_PY_MATH_SPECIAL_FUNCTIONS (0)
#define MICROPY_PY_CMATH (0)
#define MICROPY_PY_GC (1)
#define MICROPY_PY_GC_COLLECT_RETVAL (0)
#define MICROPY_PY_IO (0)
#define MICROPY_PY_IO_RESOURCE_STREAM (0)
#define MICROPY_PY_IO_FILEIO (0)
#define MICROPY_PY_IO_BYTESIO (0)
#define MICROPY_PY_IO_BUFFEREDWRITER (0)
#define MICROPY_PY_STRUCT (1)
#define MICROPY_PY_SYS (1)
#define MICROPY_PY_SYS_MAXSIZE (0)
#define MICROPY_PY_SYS_MODULES (0)
#define MICROPY_PY_SYS_EXC_INFO (0)
#define MICROPY_PY_SYS_EXIT (1)
#define MICROPY_PY_SYS_STDFILES (0)
#define MICROPY_PY_SYS_STDIO_BUFFER (0)
#define MICROPY_PY_UERRNO (0)
#define MICROPY_PY_UERRNO_ERRORCODE (0)
#define MICROPY_PY_USELECT (0)
#define MICROPY_PY_UTIME_MP_HAL (0)
#define MICROPY_PY_THREAD (0)
#define MICROPY_PY_UCTYPES (0)
#define MICROPY_PY_UZLIB (0)
#define MICROPY_PY_UJSON (0)
#define MICROPY_PY_URE (0)
#define MICROPY_PY_UHEAPQ (0)
#define MICROPY_PY_UTIMEQ (0)
#define MICROPY_PY_UHASHLIB (0)
#define MICROPY_PY_UBINASCII (1)
#define MICROPY_PY_UBINASCII_CRC32 (0)
#define MICROPY_PY_URANDOM (0)
#define MICROPY_PY_URANDOM_EXTRA_FUNCS (0)
#define MICROPY_PY_MACHINE (0)
#define MICROPY_PY_MACHINE_PULSE (0)
#define MICROPY_PY_MACHINE_I2C (0)
#define MICROPY_PY_MACHINE_SPI (0)
#define MICROPY_PY_USSL (0)
#define MICROPY_PY_USSL_FINALISER (0)
#define MICROPY_PY_WEBSOCKET (0)
#define MICROPY_PY_FRAMEBUF (0)
#define MICROPY_PY_BTREE (0)
#define MICROPY_MULTIPLE_INHERITANCE (1)
#define MICROPY_PY_ALL_SPECIAL_METHODS (0)
#define MICROPY_PY_ALL_INPLACE_SPECIAL_METHODS (0)
#define MICROPY_PY_REVERSE_SPECIAL_METHODS (0)
#define MICROPY_PY_COLLECTIONS_NAMEDTUPLE__ASDICT (0)
#define MICROPY_PY_BUILTINS_STR_UNICODE_CHECK (0)

// definitions specific to SPARC
#define MP_ENDIANNESS_BIG (1)
#define MICROPY_MAKE_POINTER_CALLABLE(p) ((void*)p)
#define MICROPY_SPARC_NUM_REG_WINDOWS (8)

// definitions specific to VM (vs cross-compiler)
#define MICROPY_ENABLE_COMPILER (0)
#define MICROPY_READER_POSIX    (0)
#define MICROPY_PERSISTENT_CODE_LOAD (1)
#define MICROPY_PERSISTENT_CODE_SAVE (0)
#define MICROPY_DYNAMIC_COMPILER (0)
#define MICROPY_USE_INTERNAL_PRINTF (0)
#define MICROPY_PY_MICROPYTHON_MEM_INFO (1)
#define MICROPY_PY___FILE__ (1)
#define MICROPY_USE_INTERNAL_ERRNO (1)

#if 0
// settings for Gaisler RTEMS
#define MICROPY_NLR_SPARC (0)
#define MICROPY_NLR_SETJMP (1)
#define MICROPY_NO_ALLOCA (0)
#else
// settings for Edisoft RTEMS
#define MICROPY_NLR_SPARC (1)
#endif

// Root pointers
#define MICROPY_PORT_ROOT_POINTERS \
    mp_obj_t rtems_script_id; \
    mp_map_t datapool_index; \

