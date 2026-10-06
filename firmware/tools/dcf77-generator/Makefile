# Bare-metal DCF-77 generator for Nucleo-F411RE. Needs only arm-none-eabi-gcc + openocd.
CC      := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
SIZE    := arm-none-eabi-size
BUILD   := _build

CFLAGS  := -mcpu=cortex-m4 -mthumb -mfloat-abi=soft -Os -g -std=c11 \
           -Wall -Wextra -Werror -ffreestanding -ffunction-sections -fdata-sections \
           -Isrc $(CFLAGS_EXTRA)
LDFLAGS := -mcpu=cortex-m4 -mthumb -nostdlib -Wl,--gc-sections -Wl,-Map=$(BUILD)/dcf77-generator.map \
           -T stm32f411re.ld
SRCS    := src/startup.c src/main.c src/dcf77_encode.c
OBJS    := $(SRCS:src/%.c=$(BUILD)/%.o)

all: $(BUILD)/dcf77-generator.elf $(BUILD)/dcf77-generator.bin
	$(SIZE) $<

$(BUILD)/%.o: src/%.c $(wildcard src/*.h) | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/dcf77-generator.elf: $(OBJS) stm32f411re.ld
	$(CC) $(LDFLAGS) $(OBJS) -lgcc -o $@

$(BUILD)/%.bin: $(BUILD)/%.elf
	$(OBJCOPY) -O binary $< $@

$(BUILD):
	mkdir -p $@

flash: $(BUILD)/dcf77-generator.elf
	openocd -f board/st_nucleo_f4.cfg \
	  -c "program $< verify reset exit"

clean:
	rm -rf $(BUILD)

.PHONY: all flash clean
