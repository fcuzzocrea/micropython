# This file is part of the MicroPython port to LEON platforms
# Copyright (c) 2015-2016 George Robotics Limited
#
# Provided to the European Space Agency as part of the project "Porting of
# MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
#
# This Makefile builds an example of a minimal LEON port.

# the location of the MicroPython LEON support code
LEON_COMMON = leon-common
LEON_COMMON_FROM_HERE = ../$(LEON_COMMON)

include ../py/mkenv.mk

# qstr definitions (must come before including py.mk)
QSTR_DEFS = $(LEON_COMMON_FROM_HERE)/qstrdefsport.h

# include py core make definitions
include ../py/py.mk

# include LEON common makefile settings
include $(LEON_COMMON_FROM_HERE)/mkenv.mk

INC =
INC += -I. -I.. -I$(BUILD) -I$(LEON_COMMON_FROM_HERE)

CFLAGS += $(INC) -Wall -Werror -ansi -std=gnu99 $(COPT) -mcpu=v8

# Use this to optimise the static hash tables
#CFLAGS += -no-integrated-cpp -B$(shell pwd)/../tools

# Debugging/Optimization
ifeq ($(DEBUG), 1)
CFLAGS += -O0 -ggdb
else
CFLAGS += -Os -DNDEBUG
endif

LIBS += -lm

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
	$(LEON_COMMON)/modrtemssem.c \
	$(LEON_COMMON)/modrtemstimer.c \
	$(LEON_COMMON)/modmem.c \

SRC_S = \
	$(LEON_COMMON)/sparcjmp.s \
	$(LEON_COMMON)/gchelper.s \

# these are the example Python scripts to compile
SRC_PY = ex1.py ex2.py ex3.py

OBJ = $(PY_O) $(addprefix $(BUILD)/, $(SRC_C:.c=.o) $(SRC_S:.s=.o) $(SRC_RTEMS:.c=.o))

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
