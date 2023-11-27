# This file is part of the MicroPython port to MPFS platforms
#
# Based on LEON port originally made by George Robotics
#
# Copyright (c) 2023 Francescodario Cuzzocrea <bosconovic@gmail.com>
#
# This Makefile builds an example of a minimal MPFS port.

# The location of the MicroPython LEON support code.
MPFS_COMMON = mpfs-common
MPFS_COMMON_FROM_HERE = ../$(MPFS_COMMON)

# Make the build directory reflect the RTEMS/toolchain version.
BUILD ?= build-$(MICROPY_RTEMS_VER)

# Include core MicroPython make environment.
include ../py/mkenv.mk

# Qstr definitions (must come before including py.mk).
QSTR_DEFS = $(MPFS_COMMON_FROM_HERE)/qstrdefsport.h

# Include core MicroPython make rules.
include ../py/py.mk

# Include MPFS common make environment.
include $(MPFS_COMMON_FROM_HERE)/mkenv.mk

SRC_C = \
	main.c \

# These are the example Python scripts to compile.
SRC_PY = pystone.py

OBJ += $(PY_O)
OBJ += $(addprefix $(BUILD)/,$(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(MPFS_COMMON_OBJ))

# Require scripts.h, containing the SRC_PY files compiled to bytecode.
main.c: $(BUILD)/scripts.h

# Include MPFS convenience rules to build the MicroPython RTEMS application.
include $(MPFS_COMMON_FROM_HERE)/mkapp.mk

include ../py/mkrules.mk
