# This file contains convenience make rules to build a MicroPython-based RTEMS application

# For now use toolchain provided libc, libm
MPFS_COMMON_SRC_C += \
	$(MPFS_COMMON)/nearbyint.c \
	$(MPFS_COMMON)/mputil.c \
	$(MPFS_COMMON)/mpvmmanage.c \
	$(MPFS_COMMON)/mphalport.c \
	$(MPFS_COMMON)/modtime.c \
	$(MPFS_COMMON)/modrtems.c \
	$(MPFS_COMMON)/modrtemstask.c \
	$(MPFS_COMMON)/modrtemsqueue.c \
	$(MPFS_COMMON)/modrtemssem.c \
	$(MPFS_COMMON)/modmem.c \
	$(MPFS_COMMON)/moddatapool.c \
	$(MPFS_COMMON)/rtems_config_common.c \
	$(MPFS_COMMON)/rtems_util.c

MPFS_COMMON_OBJ += $(SRC_RTEMS:.c=.o)
MPFS_COMMON_OBJ += $(MPFS_COMMON_SRC_C:.c=.o)
MPFS_COMMON_OBJ += $(MPFS_COMMON_SRC_S:.s=.o)

INC += -I. -I.. -I$(BUILD) -I$(MPFS_COMMON_FROM_HERE)

CFLAGS += $(INC) -Wall -Werror $(COPT)

# Debugging/Optimization
ifeq ($(DEBUG), 1)
CFLAGS += -O0 -ggdb
LDFLAGS += -Wl,-Map=build-RTEMS_6_MPFS/firmware.map
else
CFLAGS += -Os -DNDEBUG
endif

CFLAGS += $(CFLAGS_EXTRA)

LIBS += -lm

all: $(BUILD)/firmware.elf $(BUILD)/firmware.srec $(BUILD)/firmware.tab

$(BUILD)/firmware.elf: $(OBJ)
	$(ECHO) "LINK $@"
	$(Q)$(LD) $(OBJ) $(LDFLAGS) -o $@ $(LIBS)
	$(Q)$(SIZE) $@

$(BUILD)/firmware.srec: $(BUILD)/firmware.elf
	$(ECHO) "SREC $@"
	$(Q)$(OBJCOPY) -O srec $< $@

$(BUILD)/firmware.tab: $(BUILD)/firmware.elf
	$(ECHO) "TAB $@"
	$(Q)$(OBJDUMP) --syms $< > $@

$(BUILD)/firmware_combined.elf: $(BUILD)/firmware.elf
	$(Q)touch $(BUILD)/scripts.bin
	$(Q)$(OBJCOPY) --add-section .scripts=$(BUILD)/scripts.bin --change-section-address .scripts=$(MICROPY_RTEMS_MPY_MEM_BASE) --set-section-flags .scripts=contents,alloc,load,data $< $@

# General rule to build .mpy files from .py files, via mpy-cross.
$(BUILD)/%.mpy: %.py
	$(ECHO) "MPY $<"
	$(Q)$(MKDIR) -p $(dir $@)
	$(Q)../mpy-cross/mpy-cross -o $@ $<

# Build scripts.h from a set of specified .py files in $(SRC_PY).
$(BUILD)/scripts.h: $(addprefix $(BUILD)/, $(SRC_PY:.py=.mpy))
	$(ECHO) "GEN $@"
	$(Q)$(MPFS_COMMON_FROM_HERE)/mpy_package.py tohdr $^ > $@

# Rule to preprocess all the source files, producing .pp output in the $(BUILD) dir.
# We first build all the object files so that qstrs etc are generated correctly.
PREPROC_IGNORE = py/emitn%.pp mpfs-common/sparcjmp.pp mpfs-common/gchelper.pp
.PHONY: preprocess
preprocess: $(OBJ) $(filter-out $(addprefix $(BUILD)/,$(PREPROC_IGNORE)),$(OBJ:.o=.pp))