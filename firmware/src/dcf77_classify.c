#include "dcf77_classify.h"

/* Wide windows: cheap receivers stretch or shorten pulses by tens of ms. */
#define BIT0_MIN_MS 40
#define BIT0_MAX_MS 140
#define BIT1_MIN_MS 150
#define BIT1_MAX_MS 260
#define MINUTE_GAP_MIN_MS 1700
#define MINUTE_GAP_MAX_MS 2300

dcf77_pulse_t dcf77_classify_width(uint32_t width_ms)
{
    if (width_ms >= BIT0_MIN_MS && width_ms <= BIT0_MAX_MS)
    {
        return DCF77_BIT_0;
    }
    if (width_ms >= BIT1_MIN_MS && width_ms <= BIT1_MAX_MS)
    {
        return DCF77_BIT_1;
    }
    return DCF77_INVALID;
}

bool dcf77_is_minute_gap(uint32_t period_ms)
{
    return period_ms >= MINUTE_GAP_MIN_MS && period_ms <= MINUTE_GAP_MAX_MS;
}
