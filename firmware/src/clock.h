#ifndef CLOCK_H
#define CLOCK_H

#include <stdbool.h>
#include <stdint.h>

/** Civil time. wday is 0 = Monday … 6 = Sunday. */
typedef struct {
    int year;
    int month;
    int day;
    int hour;
    int min;
    int sec;
    int wday;
} clock_time;

/** Validate the fields and fill wday. Returns false if the date is impossible. */
bool clock_set(clock_time *t, int year, int month, int day, int hour, int min, int sec);

/** Parse "YYYY-MM-DD HH:MM:SS". Returns false if the text is not that form. */
bool clock_parse(const char *text, clock_time *t);

void clock_add_seconds(clock_time *t, uint32_t seconds);

/** Polish weekday name, UTF-8. */
const char *clock_wday_pl(int wday);

#endif
