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

# Make the build directory reflect the RTEMS/toolchain version.
BUILD ?= build-$(MICROPY_RTEMS_VER)

include ../py/mkenv.mk

# qstr definitions (must come before including py.mk)
QSTR_DEFS = $(LEON_COMMON_FROM_HERE)/qstrdefsport.h

# include py core make definitions
include ../py/py.mk

# include LEON common makefile settings
include $(LEON_COMMON_FROM_HERE)/mkenv.mk

SRC_C += \
	main.c \
	lib/utils/sys_stdio_mphal.c \

# List of extra sources for qstr extraction
SRC_QSTR += lib/utils/sys_stdio_mphal.c

OBJ += $(PY_O)
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(LEON_COMMON_OBJ))

# Include LEON convenience rules to build the micropython-ulab component.
include $(LEON_COMMON_FROM_HERE)/mkapp_ulab.mk

# include LEON convenience rules to build the MicroPython RTEMS application
include $(LEON_COMMON_FROM_HERE)/mkapp.mk

include ../py/mkrules.mk
