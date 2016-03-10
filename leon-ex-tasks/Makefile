# This file is part of the MicroPython port to LEON platforms
# Copyright (c) 2015-2016 George Robotics Limited
#
# Provided to the European Space Agency as part of the project "Porting of
# MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
#
# This Makefile builds the LEON port used for running the test suite.

# the location of the MIcroPython LEON support code
LEON_COMMON = leon-common
LEON_COMMON_FROM_HERE = ../$(LEON_COMMON)

include ../py/mkenv.mk

# qstr definitions (must come before including py.mk)
QSTR_DEFS = $(LEON_COMMON_FROM_HERE)/qstrdefsport.h

# include py core make definitions
include ../py/py.mk

# the toolchain needs two things:
# - the sparc-rtems compiler: (TOOL_BASE)
# - the precompiled RTEMS (RTEMS_BASE)
TOOL_BASE = /home/notroot/Download/rtems-4.8/bin
RTEMS_BASE = /home/notroot/Download/rtems-4.8/sparc-rtems/leon2

CROSS_COMPILE = $(TOOL_BASE)/sparc-rtems-
LD = $(CROSS_COMPILE)gcc
OBJDUMP = $(CROSS_COMPILE)objdump

# RTEMS lib needs
GCCSPECS = -B$(RTEMS_BASE)/lib/ -specs bsp_specs -qrtems

INC =
INC += -I. -I.. -I$(BUILD) -I$(LEON_COMMON_FROM_HERE)
INC += -I$(RTEMS_BASE)/lib/include 

CFLAGS =
CFLAGS += $(GCCSPECS)
CFLAGS += $(INC) -Wall -Werror -ansi -std=gnu99 $(COPT) -mcpu=v8

# Use this to optimise the static hash tables
CFLAGS += -no-integrated-cpp -B$(shell pwd)/../tools

# Debugging/Optimization
ifeq ($(DEBUG), 1)
CFLAGS += -O0 -ggdb
else
CFLAGS += -Os -DNDEBUG
endif

LDFLAGS =
LDFLAGS += $(GCCSPECS)
LDFLAGS += -L$(RTEMS_BASE)/lib

LIBS = -lm

SRC_C = \
	main.c \
	lib/libc/string0.c \
	$(LEON_COMMON)/mputil.c \
	$(LEON_COMMON)/mpvmmanage.c \
	$(LEON_COMMON)/mphalport.c \
	$(LEON_COMMON)/modtime.c \
	$(LEON_COMMON)/modrtems.c \
	$(LEON_COMMON)/modrtemstask.c \
	$(LEON_COMMON)/modrtemsqueue.c \
	$(LEON_COMMON)/modrtemstimer.c \

SRC_S = \
	$(LEON_COMMON)/gchelper.s \

# these are the example Python scripts to compile
# they are executed in separate tasks
SRC_PY = ex1.py ex2.py ex3.py ex4.py

OBJ = $(PY_O) $(addprefix $(BUILD)/, $(SRC_C:.c=.o) $(SRC_S:.s=.o))

all: $(BUILD)/firmware.elf $(BUILD)/firmware.srec $(BUILD)/firmware.tab $(BUILD)/scripts.srec

run: $(BUILD)/firmware.srec $(BUILD)/firmware.tab $(BUILD)/scripts.srec
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

$(BUILD)/%.mpy: %.py
	$(ECHO) "PY $<"
	$(Q)../mpy-cross/mpy-cross -o $@ $<

$(BUILD)/scripts.srec: $(addprefix $(BUILD)/, $(SRC_PY:.py=.mpy))
	$(ECHO) "MPY $@"
	$(Q)../tools/mpytool.py tosrec \
	    0x40100000 $(BUILD)/ex1.mpy \
	    0x40110000 $(BUILD)/ex2.mpy \
	    0x40120000 $(BUILD)/ex3.mpy \
	    0x40130000 $(BUILD)/ex4.mpy > $@

include ../py/mkrules.mk
