# This file is part of the MicroPython port to MPFS platforms
#
# Based on LEON port originally made by George Robotics
#
# Copyright (c) 2023 Francescodario Cuzzocrea <bosconovic@gmail.com>
#
# This Makefile builds an example of a LEON port with a VM manager.

# The location of the MicroPython MPFS support code.
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
# They are executed in separate tasks.
SRC_PY = ex1.py ex2.py ex3.py ex4.py

OBJ += $(PY_O)
OBJ += $(addprefix $(BUILD)/,$(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(MPFS_COMMON_OBJ))

# Include MPFS convenience rules to build the MicroPython RTEMS application.
include $(MPFS_COMMON_FROM_HERE)/mkapp.mk

$(BUILD)/firmware.elf: $(BUILD)/scripts.bin

# Build scripts.bin from a set of specified .py files in $(SRC_PY).
$(BUILD)/scripts.bin: $(addprefix $(BUILD)/, $(SRC_PY:.py=.mpy))
	$(ECHO) "GEN $@"
	$(Q)$(MPFS_COMMON_FROM_HERE)/mpy_package.py tobin \
		$(MICROPY_RTEMS_MPY_MEM_BASE) 0x10000 \
		$(BUILD)/ex1.mpy \
		$(BUILD)/ex2.mpy \
		$(BUILD)/ex3.mpy \
		$(BUILD)/ex4.mpy > $@

include ../py/mkrules.mk
