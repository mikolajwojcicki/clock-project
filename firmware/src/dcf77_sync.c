#include "dcf77_sync.h"

#include "board.h"
#include "dcf77_classify.h"
#include "dk_breadboard_pins.h"
#include "nrf.h"
#include "nrf_gpio.h"
#include "nrfx_gpiote.h"

#define FRAME_BITS 59
#define SECOND_MIN_MS 800u
#define SECOND_MAX_MS 1200u

typedef enum
{
    ST_OFF,
    ST_WAIT_MARKER, /* waiting for the pulse that follows a minute gap */
    ST_RECEIVE,     /* collecting the 59 bits of one frame */
    ST_WAIT_START,  /* frame 2 accepted, waiting for the pulse that starts its minute */
} state_t;

/* ponytail: one pending pulse, no queue. A second pulse before the poll loop reads
 * the first counts as an overrun and aborts the frame; the window retries. */
static volatile bool s_pending;
static volatile uint32_t s_p_start_ms;
static volatile uint32_t s_p_width_ms;
static volatile uint32_t s_p_period_ms;
static volatile uint32_t s_overruns;

/* ISR only */
static uint32_t s_rise_ms;
static uint32_t s_rise_period_ms;
static bool s_in_pulse;
static bool s_have_prev_rise;

static state_t s_state = ST_OFF;
static uint32_t s_open_ms;
static uint32_t s_seen_overruns;
static uint64_t s_bits;
static unsigned s_idx;
static bool s_have_cand;
static clock_time s_cand;
static clock_time s_accepted;

static void on_edge(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    (void)pin;
    (void)action;
    uint32_t now = board_ms();
    if (nrf_gpio_pin_read(DCF_OUT) == DCF_OUT_ACTIVE_LEVEL)
    {
        s_rise_period_ms = s_have_prev_rise ? now - s_rise_ms : 0;
        s_rise_ms = now;
        s_have_prev_rise = true;
        s_in_pulse = true;
    }
    else if (s_in_pulse)
    {
        s_in_pulse = false;
        if (s_pending)
        {
            s_overruns++;
            return;
        }
        s_p_start_ms = s_rise_ms;
        s_p_width_ms = now - s_rise_ms;
        s_p_period_ms = s_rise_period_ms;
        s_pending = true;
    }
}

void dcf77_sync_init(void)
{
    /* Already initialised is fine. */
    (void)nrfx_gpiote_init();
}

bool dcf77_sync_active(void)
{
    return s_state != ST_OFF;
}

bool dcf77_sync_open(uint32_t now_ms)
{
    if (s_state != ST_OFF)
    {
        return true;
    }
    nrfx_gpiote_in_config_t cfg = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    cfg.pull = DCF_OUT_PULL;
    if (nrfx_gpiote_in_init(DCF_OUT, &cfg, on_edge) != NRFX_SUCCESS)
    {
        return false;
    }
    s_pending = false;
    s_in_pulse = false;
    s_have_prev_rise = false;
    s_overruns = 0;
    s_seen_overruns = 0;
    s_have_cand = false;
    s_open_ms = now_ms;
    s_state = ST_WAIT_MARKER;
    /* Edge capture first, so the first pulse after power-up is not missed. */
    nrfx_gpiote_in_event_enable(DCF_OUT, true);
    nrf_gpio_pin_clear(DCF_PON);
    return true;
}

/* The only exit path: channel off, receiver off. */
static void close_window(void)
{
    nrfx_gpiote_in_event_disable(DCF_OUT);
    nrfx_gpiote_in_uninit(DCF_OUT);
    nrf_gpio_pin_set(DCF_PON);
    s_state = ST_OFF;
}

const char *dcf77_reason_name(dcf77_reason_t reason)
{
    switch (reason)
    {
    case DCF77_REASON_PARITY:
        return "parity";
    case DCF77_REASON_MARKER:
        return "marker";
    case DCF77_REASON_RANGE:
        return "range";
    case DCF77_REASON_PULSE:
        return "pulse";
    case DCF77_REASON_MISMATCH:
        return "mismatch";
    case DCF77_REASON_TIMEOUT:
        return "timeout";
    case DCF77_REASON_NONE:
        break;
    }
    return "none";
}

static unsigned field(uint64_t f, unsigned pos, unsigned n)
{
    return (unsigned)(f >> pos) & ((1u << n) - 1u);
}

static unsigned parity(uint64_t f, unsigned pos, unsigned n)
{
    return (unsigned)__builtin_popcount(field(f, pos, n)) & 1u;
}

/* Bit i of f is the pulse of second i. Fills *t (sec 0) when the reason is NONE. */
static dcf77_reason_t check_frame(uint64_t f, clock_time *t)
{
    if (field(f, 0, 1) != 0 || field(f, 20, 1) != 1 || field(f, 17, 1) == field(f, 18, 1))
    {
        return DCF77_REASON_MARKER;
    }
    /* Even parity: the parity bit is the last bit of each group. */
    if (parity(f, 21, 8) || parity(f, 29, 7) || parity(f, 36, 23))
    {
        return DCF77_REASON_PARITY;
    }
    unsigned min_u = field(f, 21, 4), min_t = field(f, 25, 3);
    unsigned hour_u = field(f, 29, 4), hour_t = field(f, 33, 2);
    unsigned day_u = field(f, 36, 4), day_t = field(f, 40, 2);
    unsigned wday = field(f, 42, 3);
    unsigned mon_u = field(f, 45, 4), mon_t = field(f, 49, 1);
    unsigned year_u = field(f, 50, 4), year_t = field(f, 54, 4);
    if (min_u > 9 || min_t > 5 || hour_u > 9 || day_u > 9 || mon_u > 9 || year_u > 9 || year_t > 9)
    {
        return DCF77_REASON_RANGE;
    }
    /* clock_set range-checks the date and fills wday (0 = Monday). DCF weekday is 1 = Monday. */
    if (!clock_set(t, 2000 + (int)(year_t * 10 + year_u), (int)(mon_t * 10 + mon_u),
                   (int)(day_t * 10 + day_u), (int)(hour_t * 10 + hour_u),
                   (int)(min_t * 10 + min_u), 0) ||
        (unsigned)t->wday + 1u != wday)
    {
        return DCF77_REASON_RANGE;
    }
    return DCF77_REASON_NONE;
}

static bool same_minute(const clock_time *a, const clock_time *b)
{
    return a->year == b->year && a->month == b->month && a->day == b->day &&
           a->hour == b->hour && a->min == b->min && a->wday == b->wday;
}

static void reject(dcf77_sync_result_t *res, dcf77_reason_t reason)
{
    res->outcome = DCF77_SYNC_REJECTED;
    res->reason = reason;
}

/* A rejected frame, or a frame that cannot pair any more: both halves are dropped. */
static void drop_frames(void)
{
    s_have_cand = false;
    s_state = ST_WAIT_MARKER;
}

static void finish_frame(dcf77_sync_result_t *res)
{
    clock_time t;
    dcf77_reason_t reason = check_frame(s_bits, &t);
    if (reason != DCF77_REASON_NONE)
    {
        reject(res, reason);
        drop_frames();
        return;
    }
    if (!s_have_cand)
    {
        s_cand = t;
        s_have_cand = true;
        s_state = ST_WAIT_MARKER;
        return;
    }
    clock_time expected = s_cand;
    clock_add_seconds(&expected, 60);
    if (!same_minute(&expected, &t))
    {
        reject(res, DCF77_REASON_MISMATCH);
        drop_frames();
        return;
    }
    s_accepted = t;
    s_state = ST_WAIT_START;
}

static void begin_frame(dcf77_pulse_t bit)
{
    s_bits = bit == DCF77_BIT_1 ? 1u : 0u;
    s_idx = 1;
    s_state = ST_RECEIVE;
}

static void on_pulse(uint32_t start_ms, uint32_t width_ms, uint32_t period_ms,
                     dcf77_sync_result_t *res)
{
    dcf77_pulse_t bit = dcf77_classify_width(width_ms);
    bool gap = dcf77_is_minute_gap(period_ms) && bit != DCF77_INVALID;
    switch (s_state)
    {
    case ST_WAIT_MARKER:
        if (gap)
        {
            begin_frame(bit);
        }
        break;
    case ST_RECEIVE:
        if (bit != DCF77_INVALID && period_ms >= SECOND_MIN_MS && period_ms <= SECOND_MAX_MS)
        {
            if (bit == DCF77_BIT_1)
            {
                s_bits |= 1ull << s_idx;
            }
            if (++s_idx == FRAME_BITS)
            {
                finish_frame(res);
            }
            break;
        }
        reject(res, DCF77_REASON_PULSE);
        drop_frames();
        if (gap)
        {
            begin_frame(bit);
        }
        break;
    case ST_WAIT_START:
        if (gap)
        {
            res->outcome = DCF77_SYNC_OK;
            res->closed = true;
            res->time = s_accepted;
            res->edge_ms = start_ms;
            close_window();
            break;
        }
        reject(res, DCF77_REASON_PULSE);
        drop_frames();
        break;
    case ST_OFF:
        break;
    }
}

void dcf77_sync_step(uint32_t now_ms, dcf77_sync_result_t *res)
{
    res->outcome = DCF77_SYNC_NONE;
    res->reason = DCF77_REASON_NONE;
    res->closed = false;
    if (s_state == ST_OFF)
    {
        return;
    }
    if (now_ms - s_open_ms >= DCF77_WINDOW_MS)
    {
        reject(res, DCF77_REASON_TIMEOUT);
        res->closed = true;
        close_window();
        return;
    }
    if (s_overruns != s_seen_overruns)
    {
        /* A pulse was lost, so the frame in progress cannot be trusted. */
        s_seen_overruns = s_overruns;
        s_pending = false;
        if (s_state != ST_WAIT_MARKER)
        {
            reject(res, DCF77_REASON_PULSE);
            drop_frames();
        }
        return;
    }
    if (!s_pending)
    {
        return;
    }
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    uint32_t start = s_p_start_ms;
    uint32_t width = s_p_width_ms;
    uint32_t period = s_p_period_ms;
    s_pending = false;
    __set_PRIMASK(primask);
    on_pulse(start, width, period, res);
}
