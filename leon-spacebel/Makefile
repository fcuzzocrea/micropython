# This file is part of the MicroPython port to LEON platforms
# Copyright (c) 2015-2016 George Robotics Limited
#
# Provided to the European Space Agency as part of the project "Porting of
# MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
#
# This Makefile builds the LEON port used for running the test suite.

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
	lib/libc/memcmp.c \
	lib/libc/memcpy.c \
	lib/libc/memmove.c \
	lib/libc/memset.c \
	lib/libc/strchr.c \
	lib/libc/strcmp.c \
	lib/libc/strlen.c \
	lib/libc/strncmp.c \
	$(LEON_COMMON)/leonprintf.c \
	$(LEON_COMMON)/nearbyint.c \
	$(LEON_COMMON)/mputil.c \
	$(LEON_COMMON)/mphalport.c \
	$(LEON_COMMON)/modtime.c \
	$(LEON_COMMON)/modrtems.c \
	$(LEON_COMMON)/modrtemstask.c \
	$(LEON_COMMON)/modrtemsqueue.c \
	$(LEON_COMMON)/modrtemssem.c \
	$(LEON_COMMON)/modmem.c \
	$(LEON_COMMON)/moddatapool.c \

SRC_S = \
	$(LEON_COMMON)/sparcjmp.s \
	$(LEON_COMMON)/gchelper.s \

OBJ = $(PY_O) $(addprefix $(BUILD)/, $(SRC_C:.c=.o) $(SRC_S:.s=.o) $(SRC_RTEMS:.c=.o))

all: $(BUILD)/firmware.elf $(BUILD)/firmware.srec $(BUILD)/firmware.tab

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

# Preprocess all the source files, producing .pp output in the $(BUILD) dir.
# We first build all the object files so that qstrs etc are generated correctly.
PREPROC_IGNORE = py/emitn%.pp leon-common/sparcjmp.pp leon-common/gchelper.pp
preprocess: $(OBJ) $(filter-out $(addprefix $(BUILD)/,$(PREPROC_IGNORE)),$(OBJ:.o=.pp))

include ../py/mkrules.mk
