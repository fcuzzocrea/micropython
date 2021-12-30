# Include core MicroPython make environment.
include ../py/mkenv.mk

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h

# Include core MicroPython make rules.
include ../py/py.mk

# Polarfire SoC BsP
PLATFORM_DIR ?= platform

# Select board
BOARD ?= mpfs-icicle-kit-es
BOARD_DIR = boards/$(BOARD)

# RISC-V TOOLS
RISCV_PATH ?= /opt/riscv64-unknown-elf/

ifeq ($(RISCV_PATH),)
CROSS_COMPILE ?= riscv64-unknown-elf-
GDB ?= riscv64-unknown-elf-gdb
else
CROSS_COMPILE = $(RISCV_PATH)/bin/riscv64-unknown-elf-
GDB = $(RISCV_PATH)/bin/riscv64-unknown-elf-gdb
endif

INC += -I.
INC += -I$(TOP)
INC += -I$(BUILD)
INC += -I$(BOARD_DIR)
INC += -I$(BOARD_DIR)/platform_config
INC += -I$(PLATFORM_DIR)

ASFLAGS_RISCV = -march=rv64imac -mabi=lp64 -mcmodel=medany -mtune=sifive-5-series -fdata-sections -ffunction-sections -fmessage-length=0 -fsigned-char -mno-save-restore -msmall-data-limit=8 -mstrict-align
ASFLAGS = $(INC) $(ASFLAGS_RISCV)
CFLAGS_RISCV = -march=rv64imac -mabi=lp64 -mcmodel=medany -mtune=sifive-5-series -fdata-sections -ffunction-sections -fmessage-length=0 -fsigned-char -mno-save-restore -msmall-data-limit=8 -mstrict-align
CFLAGS = $(INC) $(CFLAGS_RISCV) -Os -ggdb3 -gdwarf-2 -DMICROCHIP_STDIO_THRU_MMUART0
LDFLAGS_RISCV = -march=rv64imac -mabi=lp64 -mcmodel=medany
LDFLAGS = $(LDFLAGS_RISCV) -Wl,--gc-sections -Wl,-Map,$@.map -nostartfiles -nostdlib -T$(BOARD_DIR)/platform_config/linker/mpfs-lim.ld
LIBS = -Wl,--start-group -lc -lm -lgcc -Wl,--end-group

# Micropython port
SRC_C = \
	main.c

# Polarfire SoC BsP
SRC_BSP = \
	$(PLATFORM_DIR)/drivers/mss/mss_mmuart/mss_uart.c \
	$(PLATFORM_DIR)/drivers/mss/mss_sys_services/mss_sys_services.c \
	$(PLATFORM_DIR)/mpfs_hal/common/nwc/mss_cfm.c \
	$(PLATFORM_DIR)/mpfs_hal/common/nwc/mss_ddr.c \
	$(PLATFORM_DIR)/mpfs_hal/common/nwc/mss_ddr_debug.c \
	$(PLATFORM_DIR)/mpfs_hal/common/nwc/mss_ddr_test_pattern.c \
	$(PLATFORM_DIR)/mpfs_hal/common/nwc/mss_io.c \
	$(PLATFORM_DIR)/mpfs_hal/common/nwc/mss_nwc_init.c \
	$(PLATFORM_DIR)/mpfs_hal/common/nwc/mss_pll.c \
	$(PLATFORM_DIR)/mpfs_hal/common/nwc/mss_sgmii.c \
	$(PLATFORM_DIR)/mpfs_hal/common/mss_clint.c \
	$(PLATFORM_DIR)/mpfs_hal/common/mss_irq_handler_stubs.c \
	$(PLATFORM_DIR)/mpfs_hal/common/mss_l2_cache.c \
	$(PLATFORM_DIR)/mpfs_hal/common/mss_mpu.c \
	$(PLATFORM_DIR)/mpfs_hal/common/mss_mtrap.c \
	$(PLATFORM_DIR)/mpfs_hal/common/mss_peripherals.c \
	$(PLATFORM_DIR)/mpfs_hal/common/mss_plic.c \
	$(PLATFORM_DIR)/mpfs_hal/common/mss_pmp.c \
	$(PLATFORM_DIR)/mpfs_hal/common/mss_util.c \
	$(PLATFORM_DIR)/mpfs_hal/startup_gcc/system_startup.c

ASRC_BSP = \
	$(PLATFORM_DIR)/mpfs_hal/startup_gcc/mss_entry.S \
	$(PLATFORM_DIR)/mpfs_hal/startup_gcc/mss_utils.S

OBJ = $(PY_O) \
	$(addprefix $(BUILD)/, $(SRC_C:.c=.o)) \
	$(addprefix $(BUILD)/, $(ASRC_BSP:.S=.o)) \
	$(addprefix $(BUILD)/, $(SRC_BSP:.c=.o))

all: $(BUILD)/firmware.elf

$(BUILD)/firmware.elf: $(OBJ)
	$(ECHO) "LINK $@"
	$(Q)$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LIBS)
	$(Q)$(SIZE) $@

# List of sources for qstr extraction
SRC_QSTR += $(SRC_C) mpconfigport.h
# Append any auto-generated sources that are needed by sources listed in
# SRC_QSTR
SRC_QSTR_AUTO_DEPS +=

include ../py/mkrules.mk
