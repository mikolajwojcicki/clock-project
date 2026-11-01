PROJECT_NAME     := clock_pca10040
TARGETS          := nrf52832_xxaa
# make SLEEP_UART=0 builds into _build_nouart, so flag changes never reuse objects.
SLEEP_UART ?= 1
OUTPUT_DIRECTORY := _build$(if $(filter 0,$(SLEEP_UART)),_nouart)

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
  $(SDK_ROOT)/components/libraries/util/app_error_weak.c \
  $(SDK_ROOT)/components/libraries/experimental_section_vars/nrf_section_iter.c \
  $(SDK_ROOT)/components/libraries/atomic/nrf_atomic.c \
  $(SDK_ROOT)/components/libraries/atomic_fifo/nrf_atfifo.c \
  $(SDK_ROOT)/components/libraries/atomic_flags/nrf_atflags.c \
  $(SDK_ROOT)/components/libraries/fstorage/nrf_fstorage.c \
  $(SDK_ROOT)/components/libraries/fstorage/nrf_fstorage_sd.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh_ble.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh_soc.c \
  $(SDK_ROOT)/components/ble/common/ble_srv_common.c \
  $(SDK_ROOT)/components/ble/common/ble_advdata.c \
  $(SDK_ROOT)/components/ble/nrf_ble_gatt/nrf_ble_gatt.c \
  $(SDK_ROOT)/components/ble/nrf_ble_qwr/nrf_ble_qwr.c \
  $(FW_DIR)/src/board.c \
  $(FW_DIR)/src/buzzer.c \
  $(FW_DIR)/src/spi_bus.c \
  $(FW_DIR)/src/epd.c \
  $(FW_DIR)/src/alarm.c \
  $(FW_DIR)/src/sched_codec.c \
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
  $(SDK_ROOT)/components/softdevice/s112/headers \
  $(SDK_ROOT)/components/softdevice/s112/headers/nrf52 \
  $(SDK_ROOT)/components/softdevice/common \
  $(SDK_ROOT)/components/ble/common \
  $(SDK_ROOT)/components/ble/nrf_ble_gatt \
  $(SDK_ROOT)/components/ble/nrf_ble_qwr \
  $(SDK_ROOT)/components/libraries/fstorage \
  $(SDK_ROOT)/components/libraries/atomic \
  $(SDK_ROOT)/components/libraries/atomic_fifo \
  $(SDK_ROOT)/components/libraries/atomic_flags \
  $(SDK_ROOT)/components/libraries/strerror \
  $(SDK_ROOT)/components/libraries/balloc \
  $(SDK_ROOT)/components/libraries/memobj \
  $(SDK_ROOT)/components/libraries/mutex \
  $(SDK_ROOT)/components/libraries/timer \
  $(SDK_ROOT)/components/libraries/crc16 \

LIB_FILES += \

BUILD_ID := $(shell git -C $(PROJ_DIR) describe --always --dirty 2>/dev/null || echo unknown)

OPT = -O3 -g3

CFLAGS += $(OPT)
CFLAGS += -DBUILD_ID='"$(BUILD_ID)"'
CFLAGS += -DCLOCK_APP
CFLAGS += -DSLEEP_UART=$(SLEEP_UART)
CFLAGS += -DCONFIG_GPIO_AS_PINRESET
CFLAGS += -DFLOAT_ABI_HARD
CFLAGS += -DNRF52
CFLAGS += -DNRF52832_XXAA
CFLAGS += -DS112
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -DNRF_SD_BLE_API_VERSION=7
# GCC 15 flags SDK 17.1.0 nrf_fstorage.c:205 (intentional fs_data section walk).
CFLAGS += -Wno-error=array-bounds
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
ASMFLAGS += -DS112
ASMFLAGS += -DSOFTDEVICE_PRESENT
ASMFLAGS += -DNRF_SD_BLE_API_VERSION=7

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
# Optional: make flash ACM=/dev/ttyACMn picks the J-Link that owns that port.
ACM ?=
OPENOCD_SERIAL := $(if $(ACM),-c "adapter serial $(shell udevadm info -q property -n $(ACM) | sed -n 's/^ID_SERIAL_SHORT=//p')")

flash: default
	@echo Flashing: $(HEX)
	openocd -f interface/jlink.cfg $(OPENOCD_SERIAL) -c "transport select swd" -f target/nrf52.cfg \
		-c "program $(abspath $(HEX)) verify reset exit"

# nRF52832 build code G (rev 3): SystemInit locks debug access on every reset
# unless UICR APPROTECT is 0x5A, and an erase leaves it 0xFFFFFFFF.
erase:
	openocd -f interface/jlink.cfg -c "transport select swd" -f target/nrf52.cfg \
		-c "init; nrf52_recover; reset halt; flash fillw 0x10001208 0x0000005A 1; exit"

# One-time (and after make erase): S112 7.2.0 below 0x19000. The app hex does not contain it.
SD_HEX := $(SDK_ROOT)/components/softdevice/s112/hex/s112_nrf52_7.2.0_softdevice.hex
.PHONY: flash_softdevice
flash_softdevice:
	openocd -f interface/jlink.cfg $(OPENOCD_SERIAL) -c "transport select swd" -f target/nrf52.cfg \
		-c "program $(SD_HEX) verify reset exit"
