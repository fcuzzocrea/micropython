# This file is part of the MicroPython port to LEON platforms
# Copyright (c) 2015-2016 George Robotics Limited
#
# Provided to the European Space Agency as part of the project "Porting of
# MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
#
# This Makefile will build the micropython cross bytecode-compiler for LEON.

include ../py/mkenv.mk

# define main target
PROG = micropython

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h

# include py core make definitions
include ../py/py.mk

INC += -I.
INC += -I..
INC += -I$(BUILD)

# compiler settings
CWARN = -Wall -Werror
CWARN += -Wpointer-arith -Wuninitialized
CFLAGS = $(INC) $(CWARN) -ansi -std=gnu99 $(CFLAGS_MOD) $(COPT) $(CFLAGS_EXTRA)
CFLAGS += -fdata-sections -ffunction-sections -fno-asynchronous-unwind-tables -fno-strict-aliasing

# Debugging/Optimization
ifdef DEBUG
CFLAGS += -g
COPT = -O0
else
COPT = -Os #-DNDEBUG
endif

LDFLAGS = -lm $(LDFLAGS_EXTRA)
LDFLAGS += -Wl,--gc-sections

# source files
SRC_C = \
	main.c \
	gccollect.c \

OBJ = $(PY_O)
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))

include ../py/mkrules.mk
