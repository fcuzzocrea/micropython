# This file contains common settings to build for LEON using RTEMS

# To build the mpfs projects requires two components:
# - the sparc-rtems compiler, set by CROSS_COMPILE
# - the precompiled RTEMS library, set by RTEMS_BASE
# Defaults for these settings are provided below but they may need
# to be adjusted for your particular system.

# Select one of the following RTEMS versions by exporting the variable to the shell:
#export MICROPY_RTEMS_VER=RTEMS_6_MPFS

################################################################################
# Configure settings based on RTEMS version.

# Settings for RTEMS 6, MPFS
ifeq ($(MICROPY_RTEMS_VER),RTEMS_6_MPFS)
MICROPY_RTEMS_TOOLCHAIN ?= /opt/riscv-rtems6
MICROPY_RTEMS_ROOT ?= /opt/rtems/6
RTEMS_API = 6
RTEMS_CPU = riscv
RTEMS_BSP = mpfs64imafdc
RTEMS_BASE = $(MICROPY_RTEMS_ROOT)/$(RTEMS_CPU)-rtems$(RTEMS_API)/$(RTEMS_BSP)/lib
CFLAGS += -std=c99
endif

################################################################################
# General settings

# Configure the RAM address for loading .mpy files. ~4 mb after base address)
MICROPY_RTEMS_MPY_MEM_BASE = 0x1000400000
CFLAGS += -DMICROPY_RTEMS_MPY_MEM_BASE=$(MICROPY_RTEMS_MPY_MEM_BASE)

ifneq ($(RTEMS_BASE),)
LDFLAGS += -B$(RTEMS_BASE)/ -qrtems 
endif

ifneq ($(RTEMS_API),)
# RTEMS version provides a pkgconfig file to set the C and LD flags
PKG_CONFIG = $(MICROPY_RTEMS_ROOT)/lib/pkgconfig/$(RTEMS_CPU)-rtems$(RTEMS_API)-$(RTEMS_BSP).pc
ABI_FLAGS = $(shell pkg-config --cflags $(PKG_CONFIG))
LDFLAGS += $(shell pkg-config --libs $(PKG_CONFIG)) 
LDFLAGS += -march=rv64imafdc -mabi=lp64d -mcmodel=medany
CFLAGS += $(ABI_FLAGS)
CFLAGS += -DRTEMS_$(RTEMS_API)
ASFLAGS += $(ABI_FLAGS)
CROSS_COMPILE ?= $(MICROPY_RTEMS_TOOLCHAIN)/bin/$(RTEMS_CPU)-rtems$(RTEMS_API)-
endif

ifeq ($(CROSS_COMPILE),)
$(warning MICROPY_RTEMS_VER is not correctly configured, and CROSS_COMPILE is not set.)
$(warning )
$(warning Supported values for MICROPY_RTEMS_VER are:)
$(warning - RTEMS_6_MPFS)
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

$(info Building micropython-mpfs with $(MICROPY_RTEMS_VER))
$(info Using RTEMS toolchain located at $(MICROPY_RTEMS_ROOT))
