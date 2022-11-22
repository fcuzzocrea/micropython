# This file is part of the MicroPython port to LEON platforms
# Copyright (c) 2015-2016 George Robotics Limited
#
# Provided to the European Space Agency as part of the project "Porting of
# MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
#
# This file contains common settings to build for LEON using RTEMS

# To build the leon projects requires two components:
# - the sparc-rtems compiler, set by CROSS_COMPILE
# - the precompiled RTEMS library, set by RTEMS_BASE
# Defaults for these settings are provided below but they may need
# to be adjusted for your particular system.

# Select one of the following RTEMS versions by exporting the variable to the shell:
# export MICROPY_RTEMS_VER=RTEMS_4_8_EDISOFT
# export MICROPY_RTEMS_VER=RTEMS_4_8
# export MICROPY_RTEMS_VER=RTEMS_4_10
# export MICROPY_RTEMS_VER=RTEMS_4_11
# export MICROPY_RTEMS_VER=RTEMS_5_1_GR712RC
# export MICROPY_RTEMS_VER=RTEMS_5_1_GR740
# export MICROPY_RTEMS_VER=RTEMS_6_GR712RC
# export MICROPY_RTEMS_VER=RTEMS_6_GR740

################################################################################
# Configure settings based on RTEMS version.

# Settings for RTEMS 4.8 EDISOFT
# This toolchain needs a lot of extra options, and custom libc headers.
ifeq ($(MICROPY_RTEMS_VER),RTEMS_4_8_EDISOFT)
MICROPY_RTEMS_ROOT ?= /opt/rtems-4.8
CROSS_COMPILE = $(MICROPY_RTEMS_ROOT)/bin/sparc-rtems4.8-
RTEMS_BASE = $(MICROPY_RTEMS_ROOT)/sparc-rtems4.8/leon2/lib
LEON_TARGET_PLATFORM = LEON2
CFLAGS += -B$(RTEMS_BASE)
CFLAGS += -DRTEMS_4 -DRTEMS_4_8 -DRTEMS_4_8_EDISOFT
CFLAGS += -I$(LEON_COMMON_FROM_HERE)/libc-include
CFLAGS += -std=c99 -mcpu=v8
LIBS += -Wl,--start-group -lrtemscpu -lio -lno_event -lmsg -lsem -ltimer -lno_rtmon -lno_ext -lrtemsbsp -lrtemscpu -Wl,--end-group
SRC_RTEMS = leon-common/leon2serial.c

# Need gnu99 for inline assembler statements.
$(BUILD)/leon-common/sparcisr.o: CFLAGS += -std=gnu99
endif

# Settings for RTEMS 4.8 Gaisler
ifeq ($(MICROPY_RTEMS_VER),RTEMS_4_8)
MICROPY_RTEMS_ROOT ?= /opt/rtems-4.8
CROSS_COMPILE = $(MICROPY_RTEMS_ROOT)/bin/sparc-rtems-
RTEMS_BASE = $(MICROPY_RTEMS_ROOT)/sparc-rtems/leon2/lib
LEON_TARGET_PLATFORM = LEON2
CFLAGS += -B$(RTEMS_BASE)
CFLAGS += -DRTEMS_4 -DRTEMS_4_8
CFLAGS += -std=gnu99 -mcpu=v8
endif

# Settings for RTEMS 4.10
ifeq ($(MICROPY_RTEMS_VER),RTEMS_4_10)
MICROPY_RTEMS_ROOT ?= /opt/rtems-4.10
CROSS_COMPILE = $(MICROPY_RTEMS_ROOT)/bin/sparc-rtems4.10-
RTEMS_BASE = $(MICROPY_RTEMS_ROOT)/sparc-rtems4.10/leon2/lib
LEON_TARGET_PLATFORM = LEON2
CFLAGS += -DRTEMS_4 -DRTEMS_4_10
CFLAGS += -fno-strict-aliasing # suppress warnings from RTEMS 4.10
CFLAGS += -std=c99 -mcpu=v8
endif

# Settings for RTEMS 4.11
ifeq ($(MICROPY_RTEMS_VER),RTEMS_4_11)
MICROPY_RTEMS_ROOT ?= /opt/rtems-4.11
CROSS_COMPILE = $(MICROPY_RTEMS_ROOT)/bin/sparc-rtems4.11-
RTEMS_BASE = $(MICROPY_RTEMS_ROOT)/sparc-rtems4.11/leon2/lib
LEON_TARGET_PLATFORM = LEON2
CFLAGS += -DRTEMS_4 -DRTEMS_4_11
CFLAGS += -std=c99 -mcpu=v8
endif

# Settings for RTEMS 5.1, GR712RC
ifeq ($(MICROPY_RTEMS_VER),RTEMS_5_1_GR712RC)
MICROPY_RTEMS_ROOT ?= /opt/rtems-5.1-2019.07.25
LEON_TARGET_PLATFORM = LEON3
RTEMS_API = 5
RTEMS_CPU = sparc
RTEMS_BSP = gr712rc
RTEMS_SIS_FLAGS = -leon3
RTEMS_BASE = $(MICROPY_RTEMS_ROOT)/$(RTEMS_CPU)-rtems$(RTEMS_API)/$(RTEMS_BSP)/lib
CFLAGS += -std=gnu99
endif

# Settings for RTEMS 5.1, GR740
ifeq ($(MICROPY_RTEMS_VER),RTEMS_5_1_GR740)
MICROPY_RTEMS_ROOT ?= /opt/rtems-5.1-2019.07.25
LEON_TARGET_PLATFORM = LEON4
RTEMS_API = 5
RTEMS_CPU = sparc
RTEMS_BSP = gr740
RTEMS_SIS_FLAGS = -gr740 # Note: rtems-5.1-2019.07.25 SIS doesn't support GR740 so this won't work
RTEMS_BASE = $(MICROPY_RTEMS_ROOT)/$(RTEMS_CPU)-rtems$(RTEMS_API)/$(RTEMS_BSP)/lib
CFLAGS += -std=gnu99
endif

# Settings for RTEMS 6, GR712RC
ifeq ($(MICROPY_RTEMS_VER),RTEMS_6_GR712RC)
MICROPY_RTEMS_ROOT ?= /opt/rtems-6-sparc-gr712rc-smp-3
LEON_TARGET_PLATFORM = LEON3
RTEMS_API = 6
RTEMS_CPU = sparc
RTEMS_BSP = gr712rc-qual-only
RTEMS_SIS_FLAGS = -leon3
CFLAGS += -std=c99
LDFLAGS += -lc # for setjmp/longjmp
$(BUILD)/leon-common/sparcisr.o: CFLAGS += -std=gnu99
endif

# Settings for RTEMS 6, GR740
ifeq ($(MICROPY_RTEMS_VER),RTEMS_6_GR740)
MICROPY_RTEMS_ROOT ?= /opt/rtems-6-sparc-gr740-smp-3
LEON_TARGET_PLATFORM = LEON4
RTEMS_API = 6
RTEMS_CPU = sparc
RTEMS_BSP = gr740-qual-only
RTEMS_SIS_FLAGS = -gr740
CFLAGS += -std=c99
LDFLAGS += -lc # for setjmp/longjmp
endif

################################################################################
# General settings

# Set the LEON target
CFLAGS += -DLEON_TARGET_PLATFORM_$(LEON_TARGET_PLATFORM)

# Configure the RAM address for loading .mpy files leave 2MiB for firmware)
ifeq ($(findstring GR740,$(MICROPY_RTEMS_VER)),GR740)
MICROPY_RTEMS_MPY_MEM_BASE = 0x00200000
else
MICROPY_RTEMS_MPY_MEM_BASE = 0x40200000
endif
CFLAGS += -DMICROPY_RTEMS_MPY_MEM_BASE=$(MICROPY_RTEMS_MPY_MEM_BASE)

ifneq ($(RTEMS_BASE),)
# RTEMS version needs to have LD flags set explicitly
LDFLAGS += -B$(RTEMS_BASE)/ -specs bsp_specs -qrtems
endif

ifneq ($(RTEMS_API),)
# RTEMS version provides a pkgconfig file to set the C and LD flags
PKG_CONFIG = $(MICROPY_RTEMS_ROOT)/lib/pkgconfig/$(RTEMS_CPU)-rtems$(RTEMS_API)-$(RTEMS_BSP).pc
ABI_FLAGS = $(shell pkg-config --cflags $(PKG_CONFIG))
LDFLAGS += $(shell pkg-config --libs $(PKG_CONFIG))
CFLAGS += $(ABI_FLAGS)
CFLAGS += -DRTEMS_$(RTEMS_API)
CFLAGS += -Wno-error=array-bounds
#ASFLAGS += $(ABI_FLAGS)
CROSS_COMPILE ?= $(MICROPY_RTEMS_ROOT)/bin/$(RTEMS_CPU)-rtems$(RTEMS_API)-
endif

ifeq ($(CROSS_COMPILE),)
$(warning MICROPY_RTEMS_VER is not correctly configured, and CROSS_COMPILE is not set.)
$(warning )
$(warning Supported values for MICROPY_RTEMS_VER are:)
$(warning - RTEMS_4_8_EDISOFT)
$(warning - RTEMS_4_8)
$(warning - RTEMS_4_10)
$(warning - RTEMS_4_11)
$(warning - RTEMS_5_1_GR712RC)
$(warning - RTEMS_5_1_GR740)
$(warning - RTEMS_6_GR712RC)
$(warning - RTEMS_6_GR740)
$(warning )
$(warning MICROPY_RTEMS_ROOT can also be specified, otherwise it will be automatically)
$(warning set based on the value of MICROPY_RTEMS_VER.)
$(warning )
$(error Set MICROPY_RTEMS_VER to one of the above values.)
endif

# We use gcc for the linker so it gets the specs and libs right
LD = $(CC)

# objdump is needed to build the table of symbols for the emulator
OBJDUMP = $(CROSS_COMPILE)objdump

# Set path to laysim-gr740 simulator (optional component)
LAYSIM_GR740 ?= /opt/laysim-gr740/laysim-gr740-dbt-cli

$(info Building micropython-leon with $(MICROPY_RTEMS_VER))
$(info Using RTEMS toolchain located at $(MICROPY_RTEMS_ROOT))
