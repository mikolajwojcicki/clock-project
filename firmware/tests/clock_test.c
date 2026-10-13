/*
 * Host check for civil time. From the repo root:
 *   gcc -Wall -Wextra -I firmware/src firmware/tests/clock_test.c \
 *       firmware/src/clock.c -o /tmp/clock_test && /tmp/clock_test
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "clock.h"

int main(void)
{
    clock_time t;
    assert(clock_parse("2026-10-13 21:05:00", &t));
    assert(t.year == 2026 && t.month == 10 && t.day == 13);
    assert(t.hour == 21 && t.min == 5 && t.sec == 0);
    assert(t.wday == 1); /* Tuesday, Monday = 0 */
    assert(strcmp(clock_wday_pl(1), "wtorek") == 0);

    assert(!clock_parse("2026-02-29 00:00:00", &t));
    assert(clock_parse("2024-02-28 23:59:59", &t));
    clock_add_seconds(&t, 1);
    assert(t.month == 2 && t.day == 29 && t.sec == 0);
    clock_add_seconds(&t, 86400);
    assert(t.month == 3 && t.day == 1);

    assert(clock_parse("2026-12-31 23:59:59", &t));
    clock_add_seconds(&t, 1);
    assert(t.year == 2027 && t.month == 1 && t.day == 1 && t.hour == 0);

    puts("clock: all checks passed");
    return 0;
}
