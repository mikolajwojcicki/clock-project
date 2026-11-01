#include "alarm.h"

#include <string.h>

static alarm_slot s_slots[ALARM_DAYS][ALARM_SLOTS];
static bool s_active;
static bool s_start_pending; /* start time is latched on the next alarm_step */
static uint32_t s_start_ms;
static alarm_stop_reason s_stop;
static bool s_fired_valid;
static int s_fired_wday, s_fired_hour, s_fired_min;

void alarm_init(void)
{
    memset(s_slots, 0, sizeof s_slots);
    s_active = false;
    s_start_pending = false;
    s_stop = ALARM_STOP_NONE;
    s_fired_valid = false;
}

bool alarm_day_valid(const alarm_slot day[ALARM_SLOTS])
{
    for (int i = 0; i < ALARM_SLOTS; i++)
    {
        if (day[i].hour > 23 || day[i].min > 59)
        {
            return false;
        }
        for (int j = i + 1; j < ALARM_SLOTS; j++)
        {
            if (day[i].enabled && day[j].enabled && day[i].hour == day[j].hour && day[i].min == day[j].min)
            {
                return false;
            }
        }
    }
    return true;
}

bool alarm_set(int wday, int slot, int hour, int min, bool enabled)
{
    if (wday < 0 || wday >= ALARM_DAYS || slot < 0 || slot >= ALARM_SLOTS ||
        hour < 0 || hour > 23 || min < 0 || min > 59)
    {
        return false;
    }
    alarm_slot day[ALARM_SLOTS];
    memcpy(day, s_slots[wday], sizeof day);
    day[slot].hour = (uint8_t)hour;
    day[slot].min = (uint8_t)min;
    day[slot].enabled = enabled;
    if (!alarm_day_valid(day))
    {
        return false;
    }
    s_slots[wday][slot] = day[slot];
    return true;
}

bool alarm_get(int wday, int slot, alarm_slot *out)
{
    if (wday < 0 || wday >= ALARM_DAYS || slot < 0 || slot >= ALARM_SLOTS)
    {
        return false;
    }
    *out = s_slots[wday][slot];
    return true;
}

bool alarm_table_replace(const alarm_slot t[ALARM_DAYS][ALARM_SLOTS])
{
    for (int d = 0; d < ALARM_DAYS; d++)
    {
        if (!alarm_day_valid(t[d]))
        {
            return false;
        }
    }
    memcpy(s_slots, t, sizeof s_slots);
    return true;
}

void alarm_table_get(alarm_slot out[ALARM_DAYS][ALARM_SLOTS])
{
    memcpy(out, s_slots, sizeof s_slots);
}

/* Two digits at p, or -1. */
static int two_digits(const char *p)
{
    if (p[0] < '0' || p[0] > '9' || p[1] < '0' || p[1] > '9')
    {
        return -1;
    }
    return (p[0] - '0') * 10 + (p[1] - '0');
}

alarm_parse_result alarm_parse(const char *line, int *wday, int *slot)
{
    if (strncmp(line, "alarm", 5) != 0)
    {
        return ALARM_PARSE_NOT_ALARM;
    }
    const char *p = line + 5;
    if (strcmp(p, "?") == 0)
    {
        return ALARM_PARSE_LIST;
    }
    /* " <1-7> <1-8> HH:MM on|off" or the slot-1 alias " <1-7> HH:MM on|off"; 11 is the shortest. */
    if (strlen(p) < 11 || p[0] != ' ' || p[1] < '1' || p[1] > '7' || p[2] != ' ')
    {
        return ALARM_PARSE_BAD;
    }
    int day = p[1] - '1';
    int sl = 0;
    const char *r = p + 3;
    if (p[4] == ' ') /* slot form: p[3] is the slot */
    {
        if (p[3] < '1' || p[3] > '8')
        {
            return ALARM_PARSE_BAD;
        }
        sl = p[3] - '1';
        r = p + 5;
    }
    /* "HH:MM on|off" */
    if (r[2] != ':' || r[5] != ' ')
    {
        return ALARM_PARSE_BAD;
    }
    int hour = two_digits(r);
    int min = two_digits(r + 3);
    bool on;
    if (strcmp(r + 6, "on") == 0)
    {
        on = true;
    }
    else if (strcmp(r + 6, "off") == 0)
    {
        on = false;
    }
    else
    {
        return ALARM_PARSE_BAD;
    }
    if (!alarm_set(day, sl, hour, min, on))
    {
        return ALARM_PARSE_BAD;
    }
    *wday = day;
    *slot = sl;
    return ALARM_PARSE_SET;
}

bool alarm_on_minute(const clock_time *t)
{
    bool match = false;
    for (int i = 0; i < ALARM_SLOTS; i++)
    {
        const alarm_slot *s = &s_slots[t->wday][i];
        if (s->enabled && s->hour == t->hour && s->min == t->min)
        {
            match = true;
        }
    }
    if (!match)
    {
        return false;
    }
    if (s_fired_valid && s_fired_wday == t->wday && s_fired_hour == t->hour && s_fired_min == t->min)
    {
        return false;
    }
    s_fired_valid = true;
    s_fired_wday = t->wday;
    s_fired_hour = t->hour;
    s_fired_min = t->min;
    s_active = true;
    s_start_pending = true;
    return true;
}

bool alarm_on_clock_set(const clock_time *t)
{
    s_fired_valid = false;
    return alarm_on_minute(t);
}

bool alarm_step(uint32_t now_ms, bool button_pressed)
{
    if (!s_active)
    {
        return false;
    }
    if (button_pressed)
    {
        s_active = false;
        s_stop = ALARM_STOP_BUTTON;
        return false;
    }
    if (s_start_pending)
    {
        s_start_ms = now_ms;
        s_start_pending = false;
    }
    uint32_t elapsed = now_ms - s_start_ms;
    if (elapsed >= ALARM_TIMEOUT_MS)
    {
        s_active = false;
        s_stop = ALARM_STOP_TIMEOUT;
        return false;
    }
    return (elapsed % (ALARM_ON_MS + ALARM_OFF_MS)) < ALARM_ON_MS;
}

bool alarm_active(void)
{
    return s_active;
}

_Static_assert(ALARM_ON_MS == ALARM_OFF_MS, "alarm_next_ms steps by ALARM_ON_MS");

uint32_t alarm_next_ms(uint32_t now_ms)
{
    if (s_start_pending)
    {
        return now_ms;
    }
    uint32_t elapsed = now_ms - s_start_ms;
    if (elapsed >= ALARM_TIMEOUT_MS)
    {
        return now_ms;
    }
    /* ON and OFF phases change on whole periods of ALARM_ON_MS from the start. */
    return now_ms + (ALARM_ON_MS - elapsed % ALARM_ON_MS);
}

void alarm_stop_motion(void)
{
    if (s_active)
    {
        s_active = false;
        s_stop = ALARM_STOP_MOTION;
    }
}

alarm_stop_reason alarm_take_stop(void)
{
    alarm_stop_reason r = s_stop;
    s_stop = ALARM_STOP_NONE;
    return r;
}
