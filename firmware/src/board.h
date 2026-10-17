#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>
#include <stdint.h>

/** Drive DCF_PON high, EPD_CS high, SENSOR_CS high, BUZZER_EN low. */
void board_safe_state(void);

/** True while any DK button (P0.13-16, active low) is held. Stage 10 remaps this. */
bool board_any_button_pressed(void);

/** Start LFCLK from the 32.768 kHz crystal and RTC1 as a free-running timebase. */
void board_time_init(void);
uint32_t board_ms(void);

/** Busy-wait; calls app_poll() while waiting. */
void board_wait_ms(uint32_t ms);

/** Called from every wait loop. Weak no-op unless the app defines it. */
void app_poll(void);

/** Polled UART0 console on the DK virtual COM port, 115200 8N1. */
void con_init(void);
void con_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

/** One byte from the DK virtual COM port, if one is waiting. */
bool con_getc(char *out);

#endif
