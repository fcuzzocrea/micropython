#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <mpfs_hal/mss_hal.h>
#include <drivers/mss/mss_mmuart/mss_uart.h>

#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/mperrno.h"
#include "lib/utils/pyexec.h"

#include <unistd.h>
#include "py/mphal.h"

int mp_hal_stdin_rx_chr(void) {
    int c = 0;
    int r = MSS_UART_get_rx(&g_mss_uart0_lo, (uint8_t *)&c, 1);
    (void)r;
    return c;
}

void mp_hal_stdout_tx_str(const char *str) {
    mp_hal_stdout_tx_strn(str, strlen(str));
}

void mp_hal_stdout_tx_strn(const char *str, size_t len) {
    if (len > 0) {
        MSS_UART_polled_tx(&g_mss_uart0_lo, (uint8_t *)str, len);
    }
}

void mp_hal_stdout_tx_strn_cooked(const char *str, size_t len) {
    const char cr = '\r';
    if (len > 0) {
        for (size_t idx = 0; idx < len; idx++) {
            MSS_UART_polled_tx(&g_mss_uart0_lo, (uint8_t *)&str[idx], 1);
            if (str[idx] == '\n') {
                MSS_UART_polled_tx(&g_mss_uart0_lo, (uint8_t *)&cr, sizeof(cr));
            }
        }
    }
}

void do_str(const char *src, mp_parse_input_kind_t input_kind) {
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        mp_lexer_t *lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, src, strlen(src), 0);
        qstr source_name = lex->source_name;
        mp_parse_tree_t parse_tree = mp_parse(lex, input_kind);
        mp_obj_t module_fun = mp_compile(&parse_tree, source_name, MP_EMIT_OPT_BYTECODE, true);
        mp_call_function_0(module_fun);
        nlr_pop();
    } else {
        // uncaught exception
        mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
    }
}

static char *stack_top;
static char heap[1024 * 6 + 512];

void e51(void) {

    int stack_dummy;

    PLIC_init();
    __enable_irq();

    // Turn on peripherals clock
    SYSREG->SUBBLK_CLOCK_CR |=
        (SUBBLK_CLOCK_CR_MMUART0_MASK | SUBBLK_CLOCK_CR_CFM_MASK);


    // Remove peripherals from soft reset
    SYSREG->SOFT_RESET_CR &= (uint32_t) ~(
        SUBBLK_CLOCK_CR_MMUART0_MASK | SUBBLK_CLOCK_CR_CFM_MASK);

    PLIC_SetPriority_Threshold(0);
    PLIC_SetPriority(MMUART0_PLIC_77, 2);

    MSS_UART_init(&g_mss_uart0_lo,
        MSS_UART_115200_BAUD,
        MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);

    stack_top = (char *)&stack_dummy;

    gc_init(heap, heap + sizeof(heap));
    mp_init();
    mp_deinit();
}

void gc_collect(void) {

    // access all general purpose registers and SP
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

    gc_collect_start();
    gc_collect_root(regs_ptr, ((mp_uint_t)stack_top - (mp_uint_t)regs_ptr) / sizeof(mp_uint_t));
    gc_collect_end();
    gc_dump_info();
}

mp_lexer_t *mp_lexer_new_from_file(const char *filename) {
    mp_raise_OSError(MP_ENOENT);
}

mp_import_stat_t mp_import_stat(const char *path) {
    return MP_IMPORT_STAT_NO_EXIST;
}

mp_obj_t mp_builtin_open(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(mp_builtin_open_obj, 1, mp_builtin_open);

void nlr_jump_fail(void *val) {
    while (1) {
        ;
    }
}

void NORETURN __fatal_error(const char *msg) {
    while (1) {
        ;
    }
}

#ifndef NDEBUG
void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr) {
    printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    __fatal_error("Assertion failed");
}
#endif
