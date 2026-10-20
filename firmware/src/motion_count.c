#include "motion_count.h"

static uint32_t s_ring[MOTION_N]; /* timestamps of the last counted events */
static unsigned s_head;           /* next slot to write = oldest when full */
static unsigned s_len;

void motion_count_reset(void)
{
    s_head = 0;
    s_len = 0;
}

bool motion_count_event(uint32_t now_ms)
{
    if (s_len > 0)
    {
        unsigned newest = (s_head + MOTION_N - 1) % MOTION_N;
        if (now_ms - s_ring[newest] < MOTION_GAP_MS)
        {
            return false;
        }
    }
    /* Drop events that left the window. Oldest sits at head - len. */
    while (s_len > 0 &&
           now_ms - s_ring[(s_head + MOTION_N - s_len) % MOTION_N] > MOTION_WINDOW_MS)
    {
        s_len--;
    }
    s_ring[s_head] = now_ms;
    s_head = (s_head + 1) % MOTION_N;
    if (s_len < MOTION_N)
    {
        s_len++;
    }
    if (s_len == MOTION_N)
    {
        motion_count_reset();
        return true;
    }
    return false;
}

unsigned motion_count_events(void)
{
    return s_len;
}
