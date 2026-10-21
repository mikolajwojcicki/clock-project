#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>
#include <stdint.h>

/** Drive DCF_PON high, EPD_CS high, SENSOR_CS high, BUZZER_EN low. */
void board_safe_state(void);

/** True while any DK button (P0.13-16, active low) is held. Stage 10 remaps this. */
bool board_any_button_pressed(void);

/** Wake the CPU on any DK button press (GPIOTE PORT, low-power SENSE). Needs nrfx_gpiote_init. */
void board_button_wake_enable(void);

/** Start LFCLK from the 32.768 kHz crystal and RTC1 as a free-running timebase. */
void board_time_init(void);
uint32_t board_ms(void);

/** RTC1 compare channels; one per owner so nested waits do not steal the app wake. */
typedef enum
{
    BOARD_WAKE_APP,  /**< earliest app deadline, re-armed before every sleep */
    BOARD_WAKE_WAIT, /**< board_wait_ms and driver timeouts */
} board_wake_t;

/** Wake at or after target_ms (board_ms() time). Past targets make the next board_sleep return at once.
 * Targets beyond 500 s wake early; the owner re-arms. */
void board_wake_at(board_wake_t ch, uint32_t target_ms);
void board_wake_cancel(board_wake_t ch);

/** System ON sleep (WFE) until any interrupt. Call after checking for pending work. */
void board_sleep(void);

/** Sleep-wait; calls app_poll() on each wake. */
void board_wait_ms(uint32_t ms);

/** Handles pending events and arms BOARD_WAKE_APP. Weak no-op unless the app defines it. */
void app_poll(void);

/** Polled UART0 console on the DK virtual COM port, 115200 8N1. */
void con_init(void);
void con_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

/** One byte from the DK virtual COM port, if one is waiting. */
bool con_getc(char *out);

#endif
