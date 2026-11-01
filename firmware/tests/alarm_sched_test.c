/*
 * Host check for the 7 x 8 alarm table and the schedule codec. From the repo root:
 *   gcc -Wall -Wextra -I firmware/src firmware/tests/alarm_sched_test.c \
 *       firmware/src/alarm.c firmware/src/clock.c firmware/src/sched_codec.c -o /tmp/alarm_sched_test \
 *       && /tmp/alarm_sched_test
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "alarm.h"
#include "sched_codec.h"

int main(void)
{
    int d = -1, s = -1;
    alarm_init();
    assert(alarm_parse("alarm 2 3 07:30 on", &d, &s) == ALARM_PARSE_SET && d == 1 && s == 2);
    assert(alarm_parse("alarm 1 06:00 on", &d, &s) == ALARM_PARSE_SET && d == 0 && s == 0);
    assert(alarm_parse("alarm 1 9 06:00 on", &d, &s) == ALARM_PARSE_BAD);
    assert(alarm_parse("alarm 1 2 24:00 on", &d, &s) == ALARM_PARSE_BAD);
    assert(alarm_parse("alarm 1", &d, &s) == ALARM_PARSE_BAD);
    assert(alarm_parse("alarm?", &d, &s) == ALARM_PARSE_LIST);
    assert(alarm_parse("time", &d, &s) == ALARM_PARSE_NOT_ALARM);
    /* duplicate enabled HH:MM on one day rejected, table unchanged */
    assert(alarm_parse("alarm 1 2 06:00 on", &d, &s) == ALARM_PARSE_BAD);
    alarm_slot a;
    assert(alarm_get(0, 1, &a) && !a.enabled);
    /* same time on another day is fine */
    assert(alarm_parse("alarm 3 1 07:30 on", &d, &s) == ALARM_PARSE_SET);

    /* codec round trip */
    alarm_slot t[ALARM_DAYS][ALARM_SLOTS], u[ALARM_DAYS][ALARM_SLOTS];
    uint8_t buf[SCHED_LEN];
    alarm_table_get(t);
    sched_encode(t, buf);
    assert(SCHED_LEN == 113 && buf[0] == 1);
    assert(sched_decode(buf, SCHED_LEN, u) == SCHED_OK);
    assert(memcmp(t, u, sizeof t) == 0);

    /* bad writes never change the output */
    memset(u, 0x5a, sizeof u);
    alarm_slot ref[ALARM_DAYS][ALARM_SLOTS];
    memcpy(ref, u, sizeof u);
    assert(sched_decode(buf, 21, u) == SCHED_ERR_LENGTH);
    uint8_t bad[SCHED_LEN];
    memcpy(bad, buf, sizeof bad);
    bad[0] = 2;
    assert(sched_decode(bad, SCHED_LEN, u) == SCHED_ERR_VERSION);
    memcpy(bad, buf, sizeof bad);
    bad[1] = 1440 & 0xff; bad[2] = 1440 >> 8; /* minute 1440 on Monday slot 1 */
    assert(sched_decode(bad, SCHED_LEN, u) == SCHED_ERR_MINUTE);
    memcpy(bad, buf, sizeof bad);
    bad[2] |= 0x08; /* reserved bit 11 -> minute > 1439 */
    assert(sched_decode(bad, SCHED_LEN, u) == SCHED_ERR_MINUTE);
    memcpy(bad, buf, sizeof bad);
    /* Monday slot 2 = same enabled minute as slot 1 (06:00 on) */
    bad[3] = buf[1]; bad[4] = buf[2];
    assert(sched_decode(bad, SCHED_LEN, u) == SCHED_ERR_DUPLICATE);
    assert(memcmp(ref, u, sizeof u) == 0);
    puts("alarm_sched_test ok");

    return 0;
}
