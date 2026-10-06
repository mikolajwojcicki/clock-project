#include "dcf77_encode.h"

static bool leap(uint16_t y)
{
    return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
}

static uint8_t days_in_month(uint16_t y, uint8_t m)
{
    static const uint8_t d[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return (m == 2 && leap(y)) ? 29 : d[m - 1];
}

bool dcf77_time_valid(const dcf77_time_t *t)
{
    return t->year >= 2000 && t->year <= 2099 && t->month >= 1 && t->month <= 12 &&
           t->day >= 1 && t->day <= days_in_month(t->year, t->month) &&
           t->hour <= 23 && t->minute <= 59;
}

uint8_t dcf77_weekday(uint16_t y, uint8_t m, uint8_t d)
{
    /* Sakamoto: 0 = Sunday. */
    static const uint8_t t[12] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    if (m < 3) {
        y--;
    }
    uint8_t w = (uint8_t)((y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7);
    return w == 0 ? 7 : w;
}

void dcf77_time_next_minute(dcf77_time_t *t)
{
    if (++t->minute < 60) {
        return;
    }
    t->minute = 0;
    if (++t->hour < 24) {
        return;
    }
    t->hour = 0;
    if (++t->day <= days_in_month(t->year, t->month)) {
        return;
    }
    t->day = 1;
    if (++t->month <= 12) {
        return;
    }
    t->month = 1;
    t->year++;
}

/* Put `n` BCD-weighted bits of `v` at bit positions pos.. (LSB first); return parity. */
static unsigned put(uint64_t *f, unsigned pos, unsigned n, unsigned v)
{
    unsigned ones = 0;
    for (unsigned i = 0; i < n; i++) {
        if (v & (1u << i)) {
            *f |= 1ull << (pos + i);
            ones++;
        }
    }
    return ones;
}

uint64_t dcf77_encode(const dcf77_time_t *t)
{
    uint64_t f = 0;
    unsigned p;

    f |= 1ull << (t->cest ? 17 : 18);
    f |= 1ull << 20;

    p = put(&f, 21, 4, t->minute % 10) + put(&f, 25, 3, t->minute / 10);
    f |= (uint64_t)(p & 1) << 28;

    p = put(&f, 29, 4, t->hour % 10) + put(&f, 33, 2, t->hour / 10);
    f |= (uint64_t)(p & 1) << 35;

    unsigned yy = t->year % 100;
    p = put(&f, 36, 4, t->day % 10) + put(&f, 40, 2, t->day / 10) +
        put(&f, 42, 3, dcf77_weekday(t->year, t->month, t->day)) +
        put(&f, 45, 4, t->month % 10) + put(&f, 49, 1, t->month / 10) +
        put(&f, 50, 4, yy % 10) + put(&f, 54, 4, yy / 10);
    f |= (uint64_t)(p & 1) << 58;

    return f;
}
