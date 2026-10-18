#ifndef DCF77_SYNC_H
#define DCF77_SYNC_H

#include <stdbool.h>
#include <stdint.h>

#include "clock.h"

#define DCF77_WINDOW_MS 300000u

typedef enum
{
    DCF77_SYNC_NONE,
    DCF77_SYNC_OK,
    DCF77_SYNC_REJECTED,
} dcf77_sync_outcome_t;

typedef enum
{
    DCF77_REASON_NONE,
    DCF77_REASON_PARITY,   /**< a parity bit is wrong */
    DCF77_REASON_MARKER,   /**< bit 0, bit 20 or the 17/18 zone pair is wrong */
    DCF77_REASON_RANGE,    /**< a BCD field is out of range or the weekday does not fit the date */
    DCF77_REASON_PULSE,    /**< invalid pulse, missed second, or lost edge while receiving */
    DCF77_REASON_MISMATCH, /**< frames are valid but not exactly one minute apart */
    DCF77_REASON_TIMEOUT,  /**< window ended without an accepted pair */
} dcf77_reason_t;

typedef struct
{
    dcf77_sync_outcome_t outcome;
    dcf77_reason_t reason; /**< set for DCF77_SYNC_REJECTED */
    bool closed;           /**< this result ended the window (ok or timeout); PON is high again */
    clock_time time;       /**< DCF77_SYNC_OK: decoded minute, sec 0 */
    uint32_t edge_ms;      /**< DCF77_SYNC_OK: board_ms() at the start of that minute's pulse */
} dcf77_sync_result_t;

/** Prepare the GPIOTE driver. Call once, after board_safe_state(). */
void dcf77_sync_init(void);

/** Power the receiver and start decoding. Returns false if the GPIOTE channel is unavailable. */
bool dcf77_sync_open(uint32_t now_ms);

/**
 * Advance the decoder by at most one pulse. Call from the poll loop.
 * Fills *res (outcome NONE when nothing happened). The module never touches the clock.
 */
void dcf77_sync_step(uint32_t now_ms, dcf77_sync_result_t *res);

bool dcf77_sync_active(void);

const char *dcf77_reason_name(dcf77_reason_t reason);

#endif
