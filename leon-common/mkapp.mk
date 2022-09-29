# This file is part of the MicroPython port to LEON platforms
# Copyright (c) 2015-2022 George Robotics Limited
#
# Provided to the European Space Agency as part of the project "Evolutions
# of MicroPython for LEON", contract number 4000137198/22/NL/MGu/kk.
#
# This file contains convenience make rules to build a MicroPython-based RTEMS application

LEON_COMMON_SRC_C = \
	lib/libc/memcmp.c \
	lib/libc/memcpy.c \
	lib/libc/memmove.c \
	lib/libc/memset.c \
	lib/libc/strchr.c \
	lib/libc/strcmp.c \
	lib/libc/strlen.c \
	lib/libc/strncmp.c \
	$(LEON_COMMON)/sparcisr.c \
	$(LEON_COMMON)/leonprintf.c \
	$(LEON_COMMON)/leonutil.c \
	$(LEON_COMMON)/nearbyint.c \
	$(LEON_COMMON)/mputil.c \
	$(LEON_COMMON)/mpvmmanage.c \
	$(LEON_COMMON)/mphalport.c \
	$(LEON_COMMON)/modtime.c \
	$(LEON_COMMON)/modrtems.c \
	$(LEON_COMMON)/modrtemstask.c \
	$(LEON_COMMON)/modrtemsqueue.c \
	$(LEON_COMMON)/modrtemssem.c \
	$(LEON_COMMON)/modmem.c \
	$(LEON_COMMON)/moddatapool.c \
	$(LEON_COMMON)/rtems_config_common.c \
	$(LEON_COMMON)/rtems_util.c \

LEON_COMMON_SRC_S = \
	$(LEON_COMMON)/sparcjmp.s \
	$(LEON_COMMON)/gchelper.s \

LEON_COMMON_OBJ += $(SRC_RTEMS:.c=.o)
LEON_COMMON_OBJ += $(LEON_COMMON_SRC_C:.c=.o)
LEON_COMMON_OBJ += $(LEON_COMMON_SRC_S:.s=.o)

INC += -I. -I.. -I$(BUILD) -I$(LEON_COMMON_FROM_HERE)

CFLAGS += $(INC) -Wall -Werror $(COPT)

# Enable this to optimise the static hash tables
ifeq ($(LEON_OPTIMISE_HASH_TABLES), 1)
CFLAGS += -no-integrated-cpp -B$(shell pwd)/../tools
endif

# Debugging/Optimization
ifeq ($(DEBUG), 1)
CFLAGS += -O0 -ggdb
else
CFLAGS += -Os -DNDEBUG
endif

CFLAGS += $(CFLAGS_EXTRA)

LIBS += -lm

# Commands used to run firmware on leon2-emu.

LEON2_EMU_CMD_LOAD ?= '\
load /srec "$(BUILD)/firmware.srec"\n\
load /symtab "$(BUILD)/firmware.tab"\n'

LEON2_EMU_CMD_RUN ?= '\
bre %code(leon_emu_terminate) /TAG=_emu_terminate /CMD={ bre /exit/stop }\n\
set pc=0x40000000\n\
set i6=0x41000000\n\
set o6=0x40FFFE80\n\
step 300000000'

all: $(BUILD)/firmware.elf $(BUILD)/firmware.srec $(BUILD)/firmware.tab

.PHONY: run
ifeq ($(RTEMS_SIS_FLAGS),)
run: run-leon2-emu
else
run: run-sis
endif

.PHONY: run-leon2-emu
run-leon2-emu: $(BUILD)/firmware.srec $(BUILD)/firmware.tab
	$(Q)echo -e $(LEON2_EMU_CMD_LOAD) $(LEON2_EMU_CMD_LOAD_EXTRA) $(LEON2_EMU_CMD_RUN) | leon2-emu

.PHONY: run-sis
run-sis: $(BUILD)/firmware_combined.elf
	$(Q)$(CROSS_COMPILE)sis $(RTEMS_SIS_FLAGS) -dumbio -r $<

.PHONY: run-laysim-gr740
run-laysim-gr740: $(BUILD)/firmware.elf
	$(Q)$(LAYSIM_GR740) -r -core0 $<

$(BUILD)/firmware.elf: $(OBJ)
	$(ECHO) "LINK $@"
	$(Q)$(LD) $(OBJ) $(LDFLAGS) -o $@ $(LIBS)
	$(Q)$(SIZE) $@

$(BUILD)/firmware.srec: $(BUILD)/firmware.elf
	$(ECHO) "SREC $@"
	$(Q)$(OBJCOPY) -O srec $< $@

$(BUILD)/firmware.tab: $(BUILD)/firmware.elf
	$(ECHO) "TAB $@"
	$(Q)$(OBJDUMP) --syms $< > $@

$(BUILD)/firmware_combined.elf: $(BUILD)/firmware.elf
	$(Q)touch $(BUILD)/scripts.bin
	$(Q)$(OBJCOPY) --add-section .scripts=$(BUILD)/scripts.bin --change-section-address .scripts=$(MICROPY_RTEMS_MPY_MEM_BASE) --set-section-flags .scripts=contents,alloc,load,data $< $@

# General rule to build .mpy files from .py files, via mpy-cross.
$(BUILD)/%.mpy: %.py
	$(ECHO) "MPY $<"
	$(Q)$(MKDIR) -p $(dir $@)
	$(Q)../mpy-cross/mpy-cross -o $@ $<

# Build scripts.h from a set of specified .py files in $(SRC_PY).
$(BUILD)/scripts.h: $(addprefix $(BUILD)/, $(SRC_PY:.py=.mpy))
	$(ECHO) "GEN $@"
	$(Q)$(LEON_COMMON_FROM_HERE)/mpy_package.py tohdr $^ > $@

# Rule to preprocess all the source files, producing .pp output in the $(BUILD) dir.
# We first build all the object files so that qstrs etc are generated correctly.
PREPROC_IGNORE = py/emitn%.pp leon-common/sparcjmp.pp leon-common/gchelper.pp
.PHONY: preprocess
preprocess: $(OBJ) $(filter-out $(addprefix $(BUILD)/,$(PREPROC_IGNORE)),$(OBJ:.o=.pp))
