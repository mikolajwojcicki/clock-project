#include "alarm.h"

#include <string.h>

static alarm_slot s_slots[ALARM_DAYS];
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

bool alarm_set(int wday, int hour, int min, bool enabled)
{
    if (wday < 0 || wday >= ALARM_DAYS || hour < 0 || hour > 23 || min < 0 || min > 59)
    {
        return false;
    }
    s_slots[wday].hour = (uint8_t)hour;
    s_slots[wday].min = (uint8_t)min;
    s_slots[wday].enabled = enabled;
    return true;
}

bool alarm_get(int wday, alarm_slot *out)
{
    if (wday < 0 || wday >= ALARM_DAYS)
    {
        return false;
    }
    *out = s_slots[wday];
    return true;
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

alarm_parse_result alarm_parse(const char *line, int *wday)
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
    /* " <1-7> HH:MM on|off" */
    if (p[0] != ' ' || p[1] < '1' || p[1] > '7' || p[2] != ' ' || p[5] != ':' || p[8] != ' ')
    {
        return ALARM_PARSE_BAD;
    }
    int day = p[1] - '1';
    int hour = two_digits(p + 3);
    int min = two_digits(p + 6);
    bool on;
    if (strcmp(p + 9, "on") == 0)
    {
        on = true;
    }
    else if (strcmp(p + 9, "off") == 0)
    {
        on = false;
    }
    else
    {
        return ALARM_PARSE_BAD;
    }
    if (!alarm_set(day, hour, min, on))
    {
        return ALARM_PARSE_BAD;
    }
    *wday = day;
    return ALARM_PARSE_SET;
}

bool alarm_on_minute(const clock_time *t)
{
    const alarm_slot *s = &s_slots[t->wday];
    if (!s->enabled || s->hour != t->hour || s->min != t->min)
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

alarm_stop_reason alarm_take_stop(void)
{
    alarm_stop_reason r = s_stop;
    s_stop = ALARM_STOP_NONE;
    return r;
}
