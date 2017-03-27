# This file is part of the MicroPython port to LEON platforms
# Copyright (c) 2015-2016 George Robotics Limited
#
# Provided to the European Space Agency as part of the project "Porting of
# MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
#
# This file contains common settings to build for LEON2 using RTEMS

# To build the leon projects requires two components:
# - the sparc-rtems compiler, set by CROSS_COMPILE
# - the precompiled RTEMS library, set by RTEMS_BASE
# Defaults for these settings are provided below but they may need
# to be adjusted for your particular system.

# Select one of these RTEMS versions
RTEMS_VER = 4_8
#RTEMS_VER = 4_10
#RTEMS_VER = 4_11

# Settings for RTEMS 4.8
ifeq ($(RTEMS_VER),4_8)
CROSS_COMPILE = /opt/rtems-4.8/bin/sparc-rtems-
RTEMS_BASE = /opt/rtems-4.8/sparc-rtems/leon2/lib
CFLAGS += -DRTEMS_4_8 # RTEMS 4.8 requires some custom configs
endif

# Settings for RTEMS 4.10
ifeq ($(RTEMS_VER),4_10)
CROSS_COMPILE = /opt/rtems-4.10/bin/sparc-rtems4.10-
RTEMS_BASE = /opt/rtems-4.10/sparc-rtems4.10/leon2/lib
CFLAGS += -fno-strict-aliasing # suppress warnings from RTEMS 4.10
endif

# Settings for RTEMS 4.11
ifeq ($(RTEMS_VER),4_11)
CROSS_COMPILE = /opt/rtems-4.11/bin/sparc-rtems4.11-
RTEMS_BASE = /opt/rtems-4.11/sparc-rtems4.11/leon2/lib
endif

# RTEMS lib needs
LDFLAGS += -B$(RTEMS_BASE)/ -specs bsp_specs -qrtems

# We use gcc for the linker so it gets the specs and libs right
LD = $(CROSS_COMPILE)gcc

# objdump is needed to build the table of symbols for the emulator
OBJDUMP = $(CROSS_COMPILE)objdump
