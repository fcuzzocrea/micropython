# This file contains convenience make rules to build micropython-ulab.

# Configure the ulab configuration file.
CFLAGS += -DULAB_CONFIG_FILE="<mpfs_ulab_config.h>"

# Include micropython-ulab makefile fragment.
USERMOD_DIR = lib/micropython-ulab/code
include $(TOP)/lib/micropython-ulab/code/micropython.mk

# Add ulab source code to files that contain qstrs.
SRC_QSTR += $(SRC_USERMOD)

# Add ulab source code to MPFS source file list.
MPFS_COMMON_SRC_C += $(SRC_USERMOD)

# Configure additional CFLAGS based on the RTEMS toolchain version.

BUILD_ULAB = $(BUILD)/lib/micropython-ulab
