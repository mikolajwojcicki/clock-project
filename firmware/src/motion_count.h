#ifndef MOTION_COUNT_H
#define MOTION_COUNT_H

#include <stdbool.h>
#include <stdint.h>

/** Counted events needed inside the window to silence the alarm. */
#define MOTION_N 20u
/** Sliding window length. Must stay below ALARM_TIMEOUT_MS. */
#define MOTION_WINDOW_MS 30000u
/** Minimum spacing between two counted events; closer pulses are one shake. */
#define MOTION_GAP_MS 300u

/** Forget all events. Call when an alarm starts or stops. */
void motion_count_reset(void);

/**
 * Feed one INT1 pulse stamped now_ms (wraps at 2^32).
 * Returns true when this pulse is the MOTION_N-th counted event inside
 * MOTION_WINDOW_MS; the ring is then cleared.
 */
bool motion_count_event(uint32_t now_ms);

/** Counted events currently in the ring (for logging). */
unsigned motion_count_events(void);

#endif
