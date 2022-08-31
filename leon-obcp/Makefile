# This file is part of the MicroPython port to LEON platforms
# Copyright (c) 2015-2016 George Robotics Limited
#
# Provided to the European Space Agency as part of the project "Porting of
# MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
#
# This Makefile builds the OBCP example

# The location of the MicroPython LEON support code.
LEON_COMMON = leon-common
LEON_COMMON_FROM_HERE = ../$(LEON_COMMON)

# Make the build directory reflect the RTEMS/toolchain version.
BUILD ?= build-$(MICROPY_RTEMS_VER)

# Include core MicroPython make environment.
include ../py/mkenv.mk

# Qstr definitions (must come before including py.mk).
QSTR_DEFS = $(LEON_COMMON_FROM_HERE)/qstrdefsport.h qstrdefsport.h

# Include core MicroPython make rules.
include ../py/py.mk

# Include LEON common make environment.
include $(LEON_COMMON_FROM_HERE)/mkenv.mk

# The OBCP example needs printf which is not provided by the Edisoft toolchain
ifeq ($(MICROPY_RTEMS_VER),RTEMS_4_8_EDISOFT)
$(error Cannot build OBCP example with RTEMS-4.8 Edisoft (printf is reqired))
endif

SRC_C = \
	main.c \
	task_vmworker.c \
	task_tm.c \
	task_tc.c \
	task_temp.c \
	task_ground.c \
	modpowersys.c \

# These are the example Python scripts to compile.
SRC_PY = \
	task_therm.py \
	task_power.py \

OBJ += $(PY_O)
OBJ += $(addprefix $(BUILD)/,$(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(LEON_COMMON_OBJ))

# Require scripts.h, containing the SRC_PY files compiled to bytecode.
main.c: $(BUILD)/scripts.h

# Include LEON convenience rules to build the MicroPython RTEMS application.
include $(LEON_COMMON_FROM_HERE)/mkapp.mk

include ../py/mkrules.mk
