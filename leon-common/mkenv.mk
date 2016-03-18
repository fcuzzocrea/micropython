# This file is part of the MicroPython port to LEON platforms
# Copyright (c) 2015-2016 George Robotics Limited
#
# Provided to the European Space Agency as part of the project "Porting of
# MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
#
# This file contains common settings to build for LEON2 using RTEMS

# the toolchain needs two things:
# - the sparc-rtems compiler (TOOL_BASE)
# - the precompiled RTEMS library (RTEMS_BASE)
TOOL_BASE = /home/notroot/Download/rtems-4.8/bin
RTEMS_BASE = /home/notroot/Download/rtems-4.8/sparc-rtems/leon2

# cross compiler settings
CROSS_COMPILE = $(TOOL_BASE)/sparc-rtems-
LD = $(CROSS_COMPILE)gcc
OBJDUMP = $(CROSS_COMPILE)objdump

# RTEMS lib needs
GCCSPECS = -B$(RTEMS_BASE)/lib/ -specs bsp_specs -qrtems

# C flags for RTEMS
CFLAGS += $(GCCSPECS)
CFLAGS += -I$(RTEMS_BASE)/lib/include 

# LD flags for RTEMS
LDFLAGS += $(GCCSPECS)
LDFLAGS += -L$(RTEMS_BASE)/lib
