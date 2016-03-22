# This file is part of the MicroPython port to LEON platforms
# Copyright (c) 2015-2016 George Robotics Limited
#
# Provided to the European Space Agency as part of the project "Porting of
# MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
#
# This Makefile builds the OBCP example

# the location of the MicroPython LEON support code
LEON_COMMON = leon-common
LEON_COMMON_FROM_HERE = ../$(LEON_COMMON)

include ../py/mkenv.mk

# qstr definitions (must come before including py.mk)
QSTR_DEFS = $(LEON_COMMON_FROM_HERE)/qstrdefsport.h qstrdefsport.h

# include py core make definitions
include ../py/py.mk

# include LEON common makefile settings
include $(LEON_COMMON_FROM_HERE)/mkenv.mk

INC =
INC += -I. -I.. -I$(BUILD) -I$(LEON_COMMON_FROM_HERE)

CFLAGS += $(INC) -Wall -Werror -ansi -std=gnu99 $(COPT) -mcpu=v8

# Use this to optimise the static hash tables
CFLAGS += -no-integrated-cpp -B$(shell pwd)/../tools

# Debugging/Optimization
ifeq ($(DEBUG), 1)
CFLAGS += -O0 -ggdb
else
CFLAGS += -Os -DNDEBUG
endif

LIBS = -lm

# these are the Python scripts to compile into the binary
SRC_PY = \
	task_therm.py \
	task_power.py \

SRC_C = \
	main.c \
	task_vmworker.c \
	task_tm.c \
	task_tc.c \
	task_temp.c \
	task_ground.c \
	modpowersys.c \
	lib/libc/string0.c \
	$(LEON_COMMON)/mputil.c \
	$(LEON_COMMON)/mpvmmanage.c \
	$(LEON_COMMON)/mphalport.c \
	$(LEON_COMMON)/modtime.c \
	$(LEON_COMMON)/modrtems.c \
	$(LEON_COMMON)/modrtemstask.c \
	$(LEON_COMMON)/modrtemsqueue.c \
	$(LEON_COMMON)/modrtemstimer.c \
	$(LEON_COMMON)/modmem.c \
	$(LEON_COMMON)/moddatapool.c \

SRC_S = \
	$(LEON_COMMON)/gchelper.s \

OBJ = $(PY_O) $(addprefix $(BUILD)/, $(SRC_C:.c=.o) $(SRC_S:.s=.o))

all: $(BUILD)/firmware.elf $(BUILD)/firmware.srec $(BUILD)/firmware.tab

run: $(BUILD)/firmware.srec $(BUILD)/firmware.tab
	$(Q)cat cmd.txt | leon2-emu

$(BUILD)/firmware.elf: $(OBJ)
	$(ECHO) "LINK $@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $(OBJ) $(LIBS)
	$(Q)$(SIZE) $@

$(BUILD)/firmware.srec: $(BUILD)/firmware.elf
	$(ECHO) "SREC $@"
	$(Q)$(OBJCOPY) -O srec $< $@

$(BUILD)/firmware.tab: $(BUILD)/firmware.elf
	$(ECHO) "TAB $@"
	$(Q)$(OBJDUMP) --syms $< > $@   

main.c: $(BUILD)/scripts.h

$(BUILD)/%.mpy: %.py
	$(ECHO) "PY $<"
	$(Q)../mpy-cross/mpy-cross -o $@ $<

$(BUILD)/scripts.h: $(addprefix $(BUILD)/, $(SRC_PY:.py=.mpy))
	$(ECHO) "MPY $@"
	$(Q)../tools/mpytool.py tohdr $^ > $@

include ../py/mkrules.mk
