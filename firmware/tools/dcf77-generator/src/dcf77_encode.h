/* DCF-77 frame encoder: pure logic, no register access (host-testable). */
#ifndef DCF77_ENCODE_H
#define DCF77_ENCODE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint16_t year;  /* 2000..2099 */
    uint8_t month;  /* 1..12 */
    uint8_t day;    /* 1..31 */
    uint8_t hour;   /* 0..23 */
    uint8_t minute; /* 0..59 */
    bool cest;      /* true = CEST (bit 17), false = CET (bit 18) */
} dcf77_time_t;

/* True if the fields form a real calendar date and time (2000..2099). */
bool dcf77_time_valid(const dcf77_time_t *t);

/* 1 = Monday .. 7 = Sunday. */
uint8_t dcf77_weekday(uint16_t year, uint8_t month, uint8_t day);

/* Advance by one minute with hour/day/month/year rollover. Zone unchanged. */
void dcf77_time_next_minute(dcf77_time_t *t);

/* Bit n of the result is DCF-77 bit n (0..58) for the minute starting at t. */
uint64_t dcf77_encode(const dcf77_time_t *t);

#endif
