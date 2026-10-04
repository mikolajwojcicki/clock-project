/*
 * Host check for the DCF-77 pulse classifier. From the repo root:
 *   gcc -I firmware/src firmware/tests/dcf77_classify_test.c \
 *       firmware/src/dcf77_classify.c -o /tmp/dcf_test && /tmp/dcf_test
 */
#include <assert.h>
#include <stdio.h>

#include "dcf77_classify.h"

int main(void)
{
    assert(dcf77_classify_width(0) == DCF77_INVALID);
    assert(dcf77_classify_width(39) == DCF77_INVALID);
    assert(dcf77_classify_width(40) == DCF77_BIT_0);
    assert(dcf77_classify_width(100) == DCF77_BIT_0);
    assert(dcf77_classify_width(140) == DCF77_BIT_0);
    assert(dcf77_classify_width(145) == DCF77_INVALID);
    assert(dcf77_classify_width(150) == DCF77_BIT_1);
    assert(dcf77_classify_width(200) == DCF77_BIT_1);
    assert(dcf77_classify_width(260) == DCF77_BIT_1);
    assert(dcf77_classify_width(261) == DCF77_INVALID);
    assert(dcf77_classify_width(900) == DCF77_INVALID);

    assert(!dcf77_is_minute_gap(0));
    assert(!dcf77_is_minute_gap(1000));
    assert(!dcf77_is_minute_gap(1699));
    assert(dcf77_is_minute_gap(1700));
    assert(dcf77_is_minute_gap(2000));
    assert(dcf77_is_minute_gap(2300));
    assert(!dcf77_is_minute_gap(2301));
    assert(!dcf77_is_minute_gap(3000));

    puts("dcf77_classify: all checks passed");
    return 0;
}
