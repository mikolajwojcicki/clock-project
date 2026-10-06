/*
 * Host check for the DCF-77 frame encoder. From the repo root:
 *   gcc -Wall -Wextra -I firmware/tools/dcf77-generator/src \
 *       firmware/tests/dcf77_encode_test.c \
 *       firmware/tools/dcf77-generator/src/dcf77_encode.c \
 *       -o /tmp/dcf_encode_test && /tmp/dcf_encode_test
 *
 * Reference frame for 2026-10-06 20:15 CEST (Tuesday), derived by hand from
 * the DCF-77 bit table (index in the string = bit number):
 *   0..16   00000000000000000  start, weather/civil, call, announce
 *   17..20  1001               CEST=1, CET=0, leap=0, start-of-time S=1
 *   21..24  1010               minute units 5
 *   25..27  100                minute tens 1 (10)
 *   28      1                  P1: ones in 1010+100 = 3 -> odd, so 1
 *   29..32  0000               hour units 0
 *   33..34  01                 hour tens 2 (20)
 *   35      1                  P2: ones = 1 -> 1
 *   36..39  0110               day units 6
 *   40..41  00                 day tens 0
 *   42..44  010                weekday 2 (Tuesday)
 *   45..48  0000               month units 0
 *   49      1                  month tens 1 (10)
 *   50..53  0110               year units 6
 *   54..57  0100               year tens 2 (20)
 *   58      1                  P3: ones in 36..57 = 2+1+1+3 = 7 -> 1
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dcf77_encode.h"

static uint64_t from_string(const char *s)
{
    assert(strlen(s) == 59);
    uint64_t f = 0;
    for (int i = 0; i < 59; i++) {
        if (s[i] == '1') {
            f |= 1ull << i;
        }
    }
    return f;
}

static void expect_next(dcf77_time_t t, uint16_t y, uint8_t mo, uint8_t d, uint8_t h, uint8_t mi)
{
    dcf77_time_next_minute(&t);
    assert(t.year == y && t.month == mo && t.day == d && t.hour == h && t.minute == mi);
}

int main(void)
{
    const char *ref = "00000000000000000" "1001" "1010" "100" "1" "0000" "01" "1" "0110" "00" "010"
                      "0000" "1" "0110" "0100" "1";
    dcf77_time_t t = {2026, 10, 6, 20, 15, true};

    assert(dcf77_weekday(2026, 10, 6) == 2);
    assert(dcf77_encode(&t) == from_string(ref));

    /* CET sets bit 18 instead of 17 */
    t.cest = false;
    assert((dcf77_encode(&t) ^ from_string(ref)) == ((1ull << 17) | (1ull << 18)));

    /* rollovers */
    expect_next((dcf77_time_t){2026, 10, 6, 20, 15, true}, 2026, 10, 6, 20, 16);
    expect_next((dcf77_time_t){2026, 10, 6, 20, 59, true}, 2026, 10, 6, 21, 0);
    expect_next((dcf77_time_t){2026, 10, 31, 23, 59, true}, 2026, 11, 1, 0, 0);
    expect_next((dcf77_time_t){2026, 12, 31, 23, 59, false}, 2027, 1, 1, 0, 0);
    expect_next((dcf77_time_t){2028, 2, 28, 23, 59, false}, 2028, 2, 29, 0, 0);
    expect_next((dcf77_time_t){2028, 2, 29, 23, 59, false}, 2028, 3, 1, 0, 0);
    expect_next((dcf77_time_t){2027, 2, 28, 23, 59, false}, 2027, 3, 1, 0, 0);

    /* weekday advances across midnight: Tue 2026-10-06 -> Wed (3) */
    assert(dcf77_weekday(2026, 10, 7) == 3);
    assert(dcf77_weekday(2026, 10, 11) == 7); /* Sunday */
    assert(dcf77_weekday(2027, 1, 1) == 5);   /* Friday */

    /* validation */
    assert(dcf77_time_valid(&(dcf77_time_t){2028, 2, 29, 0, 0, true}));
    assert(!dcf77_time_valid(&(dcf77_time_t){2026, 2, 30, 0, 0, true}));
    assert(!dcf77_time_valid(&(dcf77_time_t){2027, 2, 29, 0, 0, true}));
    assert(!dcf77_time_valid(&(dcf77_time_t){2026, 13, 1, 0, 0, true}));
    assert(!dcf77_time_valid(&(dcf77_time_t){2026, 4, 31, 0, 0, true}));
    assert(!dcf77_time_valid(&(dcf77_time_t){2026, 1, 1, 24, 0, true}));
    assert(!dcf77_time_valid(&(dcf77_time_t){2026, 1, 1, 0, 60, true}));

    puts("dcf77 encode: all checks passed");
    return 0;
}
