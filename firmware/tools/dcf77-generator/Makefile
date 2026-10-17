# Bare-metal DCF-77 generator for Nucleo-F411RE on libopencm3 (submodule).
# Needs only arm-none-eabi-gcc + openocd. Boot code, vector table and linker script
# come from libopencm3 (generated from devices.data).
OPENCM3_DIR := ../../third_party/libopencm3
DEVICE      := stm32f411re
BUILD       := _build
OBJCOPY     := arm-none-eabi-objcopy
SIZE        := arm-none-eabi-size
PREFIX      := arm-none-eabi-
CC          := $(PREFIX)gcc
# Sets ARCH_FLAGS, CPPFLAGS, LDSCRIPT (generated in the cwd), LDLIBS, LIBDEPS.
include $(OPENCM3_DIR)/mk/genlink-config.mk

CFLAGS  := $(ARCH_FLAGS) -Os -g -std=c11 -Wall -Wextra -Werror \
           -ffunction-sections -fdata-sections $(CPPFLAGS) -Isrc $(CFLAGS_EXTRA)
LDFLAGS := $(ARCH_FLAGS) -nostartfiles -T$(LDSCRIPT) -Wl,--gc-sections \
           -Wl,-Map=$(BUILD)/dcf77-generator.map --specs=nano.specs $(LDFLAGS)
SRCS    := src/main.c src/dcf77_encode.c
OBJS    := $(SRCS:src/%.c=$(BUILD)/%.o)

all: $(BUILD)/dcf77-generator.elf $(BUILD)/dcf77-generator.bin
	$(SIZE) $<

$(BUILD)/%.o: src/%.c $(wildcard src/*.h) | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/dcf77-generator.elf: $(OBJS) $(LDSCRIPT) $(LIBDEPS)
	$(CC) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

# Library is built once, with the toolchain from PATH (no -Werror: third-party code).
$(OPENCM3_DIR)/lib/libopencm3_stm32f4.a:
	$(MAKE) -C $(OPENCM3_DIR) TARGETS=stm32/f4 lib

$(BUILD)/%.bin: $(BUILD)/%.elf
	$(OBJCOPY) -O binary $< $@

$(BUILD):
	mkdir -p $@

flash: $(BUILD)/dcf77-generator.elf
	openocd -f board/st_nucleo_f4.cfg \
	  -c "program $< verify reset exit"

clean:
	rm -rf $(BUILD) generated.$(DEVICE).ld

.PHONY: all flash clean

include $(OPENCM3_DIR)/mk/genlink-rules.mk
