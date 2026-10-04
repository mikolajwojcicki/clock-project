#ifndef DCF77_CLASSIFY_H
#define DCF77_CLASSIFY_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    DCF77_BIT_0,
    DCF77_BIT_1,
    DCF77_INVALID,
} dcf77_pulse_t;

/** Nominal widths: 100 ms = 0, 200 ms = 1. */
dcf77_pulse_t dcf77_classify_width(uint32_t width_ms);

/** Second 59 has no pulse, so the start-to-start period before second 0 is about 2 s. */
bool dcf77_is_minute_gap(uint32_t period_ms);

#endif
