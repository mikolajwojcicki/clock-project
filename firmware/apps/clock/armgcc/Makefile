PROJECT_NAME     := clock_pca10040
TARGETS          := nrf52832_xxaa
OUTPUT_DIRECTORY := _build

ifndef NRF5_SDK_DIR
$(error NRF5_SDK_DIR is not set. Export it or pass NRF5_SDK_DIR=/path/to/nRF5_SDK)
endif

SDK_ROOT := $(NRF5_SDK_DIR)
PROJ_DIR := ..
FW_DIR   := ../../..

$(OUTPUT_DIRECTORY)/nrf52832_xxaa.out: \
  LINKER_SCRIPT  := clock_gcc_nrf52.ld

SRC_FILES += \
  $(SDK_ROOT)/modules/nrfx/mdk/gcc_startup_nrf52.S \
  $(SDK_ROOT)/modules/nrfx/mdk/system_nrf52.c \
  $(SDK_ROOT)/components/libraries/fifo/app_fifo.c \
  $(SDK_ROOT)/components/libraries/uart/app_uart_fifo.c \
  $(SDK_ROOT)/components/libraries/util/app_util_platform.c \
  $(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_uart.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/prs/nrfx_prs.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_uart.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_uarte.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_spim.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_pwm.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_rtc.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_clock.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_gpiote.c \
  $(FW_DIR)/src/board.c \
  $(FW_DIR)/src/buzzer.c \
  $(FW_DIR)/src/spi_bus.c \
  $(FW_DIR)/src/epd.c \
  $(FW_DIR)/src/alarm.c \
  $(FW_DIR)/src/motion_count.c \
  $(FW_DIR)/src/lis3dh.c \
  $(FW_DIR)/src/clock.c \
  $(FW_DIR)/src/clock_face.c \
  $(FW_DIR)/src/dcf77_classify.c \
  $(FW_DIR)/src/dcf77_sync.c \
  $(PROJ_DIR)/src/main.c \

INC_FOLDERS += \
  $(PROJ_DIR)/config \
  $(FW_DIR)/src \
  $(FW_DIR)/boards \
  $(SDK_ROOT)/modules/nrfx \
  $(SDK_ROOT)/modules/nrfx/mdk \
  $(SDK_ROOT)/modules/nrfx/hal \
  $(SDK_ROOT)/integration/nrfx \
  $(SDK_ROOT)/integration/nrfx/legacy \
  $(SDK_ROOT)/modules/nrfx/drivers/include \
  $(SDK_ROOT)/components/libraries/fifo \
  $(SDK_ROOT)/components/libraries/uart \
  $(SDK_ROOT)/components/libraries/log \
  $(SDK_ROOT)/components/libraries/log/src \
  $(SDK_ROOT)/components/libraries/delay \
  $(SDK_ROOT)/components/libraries/experimental_section_vars \
  $(SDK_ROOT)/components/toolchain/cmsis/include \
  $(SDK_ROOT)/components/libraries/util \
  $(SDK_ROOT)/components/drivers_nrf/nrf_soc_nosd \

LIB_FILES += \

BUILD_ID := $(shell git -C $(PROJ_DIR) describe --always --dirty 2>/dev/null || echo unknown)

OPT = -O3 -g3

CFLAGS += $(OPT)
CFLAGS += -DBUILD_ID='"$(BUILD_ID)"'
CFLAGS += -DCLOCK_APP
CFLAGS += -DCONFIG_GPIO_AS_PINRESET
CFLAGS += -DFLOAT_ABI_HARD
CFLAGS += -DNRF52
CFLAGS += -DNRF52832_XXAA
CFLAGS += -DNRF52_PAN_74
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += -Wall -Werror
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin -fshort-enums

CXXFLAGS += $(OPT)
ASMFLAGS += -g3
ASMFLAGS += -mcpu=cortex-m4
ASMFLAGS += -mthumb -mabi=aapcs
ASMFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
ASMFLAGS += -DCONFIG_GPIO_AS_PINRESET
ASMFLAGS += -DFLOAT_ABI_HARD
ASMFLAGS += -DNRF52
ASMFLAGS += -DNRF52832_XXAA
ASMFLAGS += -DNRF52_PAN_74

LDFLAGS += $(OPT)
LDFLAGS += -mthumb -mabi=aapcs -L$(SDK_ROOT)/modules/nrfx/mdk -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
LDFLAGS += -Wl,--gc-sections
LDFLAGS += --specs=nano.specs

nrf52832_xxaa: CFLAGS += -D__HEAP_SIZE=2048
nrf52832_xxaa: CFLAGS += -D__STACK_SIZE=8192
nrf52832_xxaa: ASMFLAGS += -D__HEAP_SIZE=2048
nrf52832_xxaa: ASMFLAGS += -D__STACK_SIZE=8192

LIB_FILES += -lc -lnosys -lm

.PHONY: default help FORCE

default: nrf52832_xxaa

help:
	@echo following targets are available:
	@echo		nrf52832_xxaa
	@echo		flash      - program via OpenOCD (on-board J-Link)
	@echo		erase      - mass-erase via OpenOCD
	@echo		clean

TEMPLATE_PATH := $(SDK_ROOT)/components/toolchain/gcc

include $(TEMPLATE_PATH)/Makefile.common

$(foreach target, $(TARGETS), $(call define_target, $(target)))

$(OUTPUT_DIRECTORY)/nrf52832_xxaa/main.c.o: FORCE
FORCE:

.PHONY: flash erase

HEX := $(OUTPUT_DIRECTORY)/nrf52832_xxaa.hex

flash: default
	@echo Flashing: $(HEX)
	openocd -f interface/jlink.cfg -c "transport select swd" -f target/nrf52.cfg \
		-c "program $(abspath $(HEX)) verify reset exit"

erase:
	openocd -f interface/jlink.cfg -c "transport select swd" -f target/nrf52.cfg \
		-c "init; nrf52_recover; exit"
