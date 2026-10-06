/**
 * nRF52 DK (PCA10040) breadboard prototype pin map.
 *
 * Source of truth: hardware/docs/breadboard-prototype-guide.md section 6.
 * P0.22 to P0.31 are low-drive pins: keep signals there at or below 10 kHz.
 */
#ifndef DK_BREADBOARD_PINS_H
#define DK_BREADBOARD_PINS_H

#include "nrf_gpio.h"

/* Shared SPI bus (display + LIS3DH) */
#define SPI_SCK      3
#define SPI_MOSI     4
#define SPI_MISO     2

/* Waveshare 2.13" e-paper */
#define EPD_CS       11
#define EPD_DC       12
#define EPD_RST      28
#define EPD_BUSY     29

/* LIS3DH */
#define SENSOR_CS    30
#define SENSOR_INT1  23

/* DCF-1060N-800 receiver; PON is active low */
#define DCF_PON      24
#define DCF_OUT      25
#define DCF_OUT_ACTIVE_LEVEL 1
/* DCF-1060N-800 OUT is push-pull: it held low with the internal pull-up on, so no pull. */
#define DCF_OUT_PULL NRF_GPIO_PIN_NOPULL

/* Buzzer transistor base (through 1 kOhm) */
#define BUZZER_EN    31

/* DK on-board parts (reserved pins, used only for operator I/O) */
#define DK_BUTTON1   13
#define DK_BUTTON2   14
#define DK_BUTTON3   15
#define DK_BUTTON4   16
#define DK_UART_TX   6
#define DK_UART_RX   8

#endif
