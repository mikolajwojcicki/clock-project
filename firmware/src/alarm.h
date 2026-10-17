#ifndef ALARM_H
#define ALARM_H

#include <stdbool.h>
#include <stdint.h>

#include "clock.h"

#define ALARM_DAYS 7
#define ALARM_ON_MS 1000u
#define ALARM_OFF_MS 1000u
#define ALARM_TIMEOUT_MS 60000u

/** One slot per weekday, index = clock_time.wday (0 = Monday). */
typedef struct {
    uint8_t hour;
    uint8_t min;
    bool enabled;
} alarm_slot;

typedef enum {
    ALARM_PARSE_NOT_ALARM, /**< line does not start with "alarm" */
    ALARM_PARSE_SET,       /**< table updated, *wday holds the slot */
    ALARM_PARSE_LIST,      /**< "alarm?" */
    ALARM_PARSE_BAD        /**< starts with "alarm" but invalid, table unchanged */
} alarm_parse_result;

typedef enum {
    ALARM_STOP_NONE,
    ALARM_STOP_BUTTON,
    ALARM_STOP_TIMEOUT
} alarm_stop_reason;

/** Disable all slots and stop any active alarm. */
void alarm_init(void);

/** Returns false (table unchanged) if wday, hour or min is out of range. */
bool alarm_set(int wday, int hour, int min, bool enabled);
bool alarm_get(int wday, alarm_slot *out);

/** Parse "alarm <1-7> HH:MM on|off" or "alarm?". 1 = Monday. */
alarm_parse_result alarm_parse(const char *line, int *wday);

/**
 * Call when the clock minute changes. Starts the alarm if the slot for t matches,
 * unless this same wday/HH:MM already fired. A match while sounding restarts the timeout.
 * Returns true if the alarm (re)started.
 */
bool alarm_on_minute(const clock_time *t);

/** Call when the clock is set. Like alarm_on_minute, but forgets the last fired minute. */
bool alarm_on_clock_set(const clock_time *t);

/** Advance the 1 s on/off pattern. Returns whether the buzzer should sound now. */
bool alarm_step(uint32_t now_ms, bool button_pressed);

bool alarm_active(void);

/** Returns why the alarm stopped since the last call, then clears it. */
alarm_stop_reason alarm_take_stop(void);

#endif
