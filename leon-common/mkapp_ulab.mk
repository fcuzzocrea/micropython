# This file is part of the MicroPython port to LEON platforms
# Copyright (c) 2015-2022 George Robotics Limited
#
# Provided to the European Space Agency as part of the project "Evolutions
# of MicroPython for LEON", contract number 4000137198/22/NL/MGu/kk.
#
# This file contains convenience make rules to build micropython-ulab.

# Configure the ulab configuration file.
CFLAGS += -DULAB_CONFIG_FILE="<leon_ulab_config.h>"

# Include micropython-ulab makefile fragment.
USERMOD_DIR = lib/micropython-ulab/code
include $(TOP)/lib/micropython-ulab/code/micropython.mk

# Add ulab source code to files that contain qstrs.
SRC_QSTR += $(SRC_USERMOD)

# Add ulab source code to LEON source file list.
LEON_COMMON_SRC_C += $(SRC_USERMOD)

# Configure additional CFLAGS based on the RTEMS toolchain version.

BUILD_ULAB = $(BUILD)/lib/micropython-ulab

ifeq ($(MICROPY_RTEMS_VER),RTEMS_4_8_EDISOFT)
# transform.c includes sys/types.h, but this causes a few compilation
# problems due to the RTEMS toolchain limitations:
#  - wint_t and ptrdiff_t are not defined
#  - struct timeval and struct timespec get multiple definitions
# So make the include of sys/types.h do nothing by defining its header guard macro
$(BUILD_ULAB)/code/numpy/transform.o: CFLAGS += -D_SYS_TYPES_H
endif

ifeq ($(findstring RTEMS_4_8,$(MICROPY_RTEMS_VER)),RTEMS_4_8)
$(BUILD_ULAB)/code/numpy/create.o: CFLAGS += -Wno-uninitialized
$(BUILD_ULAB)/code/numpy/transform.o: CFLAGS += -Wno-uninitialized
endif

ifeq ($(MICROPY_RTEMS_VER),RTEMS_4_10)
$(BUILD_ULAB)/code/numpy/create.o: CFLAGS += -Wno-uninitialized
$(BUILD_ULAB)/code/numpy/numerical.o: CFLAGS += -Wno-parentheses
$(BUILD_ULAB)/code/numpy/transform.o: CFLAGS += -Wno-uninitialized
endif

ifeq ($(MICROPY_RTEMS_VER),RTEMS_4_11)
$(BUILD_ULAB)/code/numpy/transform.o: CFLAGS += -Wno-uninitialized
endif

ifeq ($(findstring RTEMS_5_1,$(MICROPY_RTEMS_VER)),RTEMS_5_1)
$(BUILD_ULAB)/code/%.o: CFLAGS += -Wno-missing-prototypes
endif
