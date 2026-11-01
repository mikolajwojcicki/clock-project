#ifndef ALARM_H
#define ALARM_H

#include <stdbool.h>
#include <stdint.h>

#include "clock.h"

#define ALARM_DAYS 7
#define ALARM_SLOTS 8
#define ALARM_ON_MS 1000u
#define ALARM_OFF_MS 1000u
#define ALARM_TIMEOUT_MS 60000u

/** One alarm. The table is [weekday][slot]; weekday index = clock_time.wday (0 = Monday). */
typedef struct {
    uint8_t hour;
    uint8_t min;
    bool enabled;
} alarm_slot;

typedef enum {
    ALARM_PARSE_NOT_ALARM, /**< line does not start with "alarm" */
    ALARM_PARSE_SET,       /**< table updated, *wday and *slot hold the slot */
    ALARM_PARSE_LIST,      /**< "alarm?" */
    ALARM_PARSE_BAD        /**< starts with "alarm" but invalid, table unchanged */
} alarm_parse_result;

typedef enum {
    ALARM_STOP_NONE,
    ALARM_STOP_BUTTON,
    ALARM_STOP_MOTION,
    ALARM_STOP_TIMEOUT
} alarm_stop_reason;

/** Disable all slots and stop any active alarm. */
void alarm_init(void);

/**
 * Returns false (table unchanged) if wday, slot, hour or min is out of range, or if the
 * slot would be enabled with the same HH:MM as another enabled slot on that weekday.
 */
bool alarm_set(int wday, int slot, int hour, int min, bool enabled);
bool alarm_get(int wday, int slot, alarm_slot *out);

/** True if every slot of one weekday has hour < 24, min < 60 and no two enabled slots share HH:MM. */
bool alarm_day_valid(const alarm_slot day[ALARM_SLOTS]);

/** Replace the whole table, or nothing: false (table unchanged) if any weekday is not valid. */
bool alarm_table_replace(const alarm_slot t[ALARM_DAYS][ALARM_SLOTS]);
void alarm_table_get(alarm_slot out[ALARM_DAYS][ALARM_SLOTS]);

/**
 * Parse "alarm <1-7> <1-8> HH:MM on|off", "alarm <1-7> HH:MM on|off" (slot 1) or "alarm?".
 * 1 = Monday. On SET, *wday and *slot are zero-based.
 */
alarm_parse_result alarm_parse(const char *line, int *wday, int *slot);

/**
 * Call when the clock minute changes. Starts the alarm if a slot for t matches,
 * unless this same wday/HH:MM already fired. A match while sounding restarts the timeout.
 * Returns true if the alarm (re)started. Writing the table never calls this.
 */
bool alarm_on_minute(const clock_time *t);

/** Call when the clock is set. Like alarm_on_minute, but forgets the last fired minute. */
bool alarm_on_clock_set(const clock_time *t);

/** Advance the 1 s on/off pattern. Returns whether the buzzer should sound now. */
bool alarm_step(uint32_t now_ms, bool button_pressed);

bool alarm_active(void);

/** While active: the board_ms() time of the next buzzer phase change or timeout check.
 * Assumes ALARM_ON_MS == ALARM_OFF_MS. */
uint32_t alarm_next_ms(uint32_t now_ms);

/** Stop a sounding alarm because enough motion events arrived. No-op if not active. */
void alarm_stop_motion(void);

/** Returns why the alarm stopped since the last call, then clears it. */
alarm_stop_reason alarm_take_stop(void);

#endif
