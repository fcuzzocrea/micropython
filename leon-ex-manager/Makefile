# This file is part of the MicroPython port to LEON platforms
# Copyright (c) 2015-2016 George Robotics Limited
#
# Provided to the European Space Agency as part of the project "Porting of
# MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
#
# This Makefile builds an example of a LEON port with a VM manager.

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

SRC_C = \
	main.c \

# these are the example Python scripts to compile
SRC_PY = ex1.py ex2.py

OBJ += $(PY_O)
OBJ += $(addprefix $(BUILD)/,$(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(LEON_COMMON_OBJ))

# Require scripts.h, containing the SRC_PY files compiled to bytecode.
main.c: $(BUILD)/scripts.h

# Include LEON convenience rules to build the MicroPython RTEMS application.
include $(LEON_COMMON_FROM_HERE)/mkapp.mk

# Include MicroPython make rules.
include ../py/mkrules.mk
