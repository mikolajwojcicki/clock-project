#include "clock.h"

#include <stddef.h>

static bool leap(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static int month_days(int year, int month)
{
    static const int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && leap(year))
    {
        return 29;
    }
    return days[month];
}

/* Sakamoto, Sunday = 0. */
static int sunday0(int year, int month, int day)
{
    static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    if (month < 3)
    {
        year--;
    }
    int w = year + year / 4 - year / 100 + year / 400 + t[month - 1] + day;
    return w % 7;
}

bool clock_set(clock_time *t, int year, int month, int day, int hour, int min, int sec)
{
    if (year < 2000 || year > 2099 || month < 1 || month > 12)
    {
        return false;
    }
    if (day < 1 || day > month_days(year, month))
    {
        return false;
    }
    if (hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59)
    {
        return false;
    }
    t->year = year;
    t->month = month;
    t->day = day;
    t->hour = hour;
    t->min = min;
    t->sec = sec;
    /* Monday = 0. */
    t->wday = (sunday0(year, month, day) + 6) % 7;
    return true;
}

static int two(const char *s, int *out)
{
    if (s[0] < '0' || s[0] > '9' || s[1] < '0' || s[1] > '9')
    {
        return 0;
    }
    *out = (s[0] - '0') * 10 + (s[1] - '0');
    return 2;
}

static int four(const char *s, int *out)
{
    int hi;
    int lo;
    if (!two(s, &hi) || !two(s + 2, &lo))
    {
        return 0;
    }
    *out = hi * 100 + lo;
    return 4;
}

bool clock_parse(const char *text, clock_time *t)
{
    int year, month, day, hour, min, sec;
    if (!four(text, &year) || text[4] != '-')
    {
        return false;
    }
    if (!two(text + 5, &month) || text[7] != '-')
    {
        return false;
    }
    if (!two(text + 8, &day) || text[10] != ' ')
    {
        return false;
    }
    if (!two(text + 11, &hour) || text[13] != ':')
    {
        return false;
    }
    if (!two(text + 14, &min) || text[16] != ':')
    {
        return false;
    }
    if (!two(text + 17, &sec) || text[19] != '\0')
    {
        return false;
    }
    return clock_set(t, year, month, day, hour, min, sec);
}

void clock_add_seconds(clock_time *t, uint32_t seconds)
{
    uint32_t sec = (uint32_t)t->sec + seconds;
    t->sec = (int)(sec % 60u);
    uint32_t min = (uint32_t)t->min + sec / 60u;
    t->min = (int)(min % 60u);
    uint32_t hour = (uint32_t)t->hour + min / 60u;
    t->hour = (int)(hour % 24u);
    uint32_t days = hour / 24u;
    while (days > 0)
    {
        days--;
        t->day++;
        t->wday = (t->wday + 1) % 7;
        if (t->day > month_days(t->year, t->month))
        {
            t->day = 1;
            t->month++;
            if (t->month > 12)
            {
                t->month = 1;
                t->year++;
            }
        }
    }
}

const char *clock_wday_pl(int wday)
{
    static const char *names[] = {
        "poniedziałek",
        "wtorek",
        "środa",
        "czwartek",
        "piątek",
        "sobota",
        "niedziela",
    };
    if (wday < 0 || wday > 6)
    {
        return "?";
    }
    return names[wday];
}
