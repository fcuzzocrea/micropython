# This file is part of the MicroPython port to LEON platforms
# Copyright (c) 2015-2016 George Robotics Limited
#
# Provided to the European Space Agency as part of the project "Porting of
# MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
#
# This Makefile builds an example of a LEON port with multiple tasks.

# The location of the MicroPython LEON support code.
LEON_COMMON = leon-common
LEON_COMMON_FROM_HERE = ../$(LEON_COMMON)

# Make the build directory reflect the RTEMS/toolchain version.
BUILD ?= build-$(MICROPY_RTEMS_VER)

# Include core MicroPython make environment.
include ../py/mkenv.mk

# Qstr definitions (must come before including py.mk).
QSTR_DEFS = $(LEON_COMMON_FROM_HERE)/qstrdefsport.h

# Include core MicroPython make rules.
include ../py/py.mk

# Include LEON common make environment.
include $(LEON_COMMON_FROM_HERE)/mkenv.mk

SRC_C = \
	main.c \

# These are the example Python scripts to compile.
# They are executed in separate tasks.
SRC_PY = ex1.py ex2.py ex3.py ex4.py

OBJ += $(PY_O)
OBJ += $(addprefix $(BUILD)/,$(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(LEON_COMMON_OBJ))

LEON2_EMU_CMD_LOAD_EXTRA = 'load /srec "$(BUILD)/scripts.srec"\n'

# Include LEON convenience rules to build the MicroPython RTEMS application.
include $(LEON_COMMON_FROM_HERE)/mkapp.mk

$(BUILD)/firmware.elf: $(BUILD)/scripts.srec $(BUILD)/scripts.bin

# Build scripts.srec from a set of specified .py files in $(SRC_PY).
$(BUILD)/scripts.srec: $(addprefix $(BUILD)/, $(SRC_PY:.py=.mpy))
	$(ECHO) "GEN $@"
	$(Q)$(LEON_COMMON_FROM_HERE)/mpy_package.py tosrec \
	    0x40100000 $(BUILD)/ex1.mpy \
	    0x40110000 $(BUILD)/ex2.mpy \
	    0x40120000 $(BUILD)/ex3.mpy \
	    0x40130000 $(BUILD)/ex4.mpy > $@

# Build scripts.bin from a set of specified .py files in $(SRC_PY).
$(BUILD)/scripts.bin: $(addprefix $(BUILD)/, $(SRC_PY:.py=.mpy))
	$(ECHO) "GEN $@"
	$(Q)$(LEON_COMMON_FROM_HERE)/mpy_package.py tobin \
	    0x40100000 $(BUILD)/ex1.mpy \
	    0x40110000 $(BUILD)/ex2.mpy \
	    0x40120000 $(BUILD)/ex3.mpy \
	    0x40130000 $(BUILD)/ex4.mpy > $@

include ../py/mkrules.mk
