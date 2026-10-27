#include "board.h"
#include "dcf77_sync.h"
#include "alarm.h"
#include "buzzer.h"
#include "clock.h"
#include "clock_face.h"
#include "epd.h"
#include "spi_bus.h"
#include "dk_breadboard_pins.h"
#include "nrf_gpio.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "lis3dh.h"
#include "motion_count.h"
#include "nrfx_gpiote.h"

static clock_time s_time;

#define DCF_RETRIES 3
#define DCF_RETRY_MS 3600000u
#define DCF_LATE_MS 300000u

static bool s_dcf_pending;
static uint32_t s_dcf_pending_ms;
static uint8_t s_dcf_retries_left;
static bool s_dcf_retry_wait;
static uint32_t s_dcf_retry_ms;
static bool s_set;
static uint32_t s_last_ms;
static int s_drawn_min = -1;
static bool s_draw;
static bool s_drawing;
static char s_line[24];
static unsigned s_line_len;

static bool face_pixel(uint32_t x, uint32_t y, void *ctx);

static void draw(void)
{
    s_drawing = true;
    uint32_t t0 = board_ms();
    bool ok = epd_refresh(face_pixel, &s_time);
    con_printf("display %s %lu ms\n", ok ? "ok" : "FAIL", (unsigned long)(board_ms() - t0));
    s_drawn_min = s_time.min;
    s_draw = false;
    s_drawing = false;
}

static void announce_alarm(bool started)
{
    if (started)
    {
        con_printf("ALARM %02d:%02d %s\n", s_time.hour, s_time.min, clock_wday_pl(s_time.wday));
    }
}

static void print_slot(int wday)
{
    alarm_slot a;
    alarm_get(wday, &a);
    con_printf("alarm %d %02d:%02d %s\n", wday + 1, a.hour, a.min, a.enabled ? "on" : "off");
}

static void accept_line(void)
{
    s_line[s_line_len] = '\0';
    s_line_len = 0;
    int wday = 0;
    switch (alarm_parse(s_line, &wday))
    {
    case ALARM_PARSE_SET:
        print_slot(wday);
        return;
    case ALARM_PARSE_LIST:
        for (int d = 0; d < ALARM_DAYS; d++)
        {
            print_slot(d);
        }
        return;
    case ALARM_PARSE_BAD:
        con_printf("usage: alarm <1-7> HH:MM on|off, alarm?\n");
        return;
    case ALARM_PARSE_NOT_ALARM:
        break;
    }
    if (strcmp(s_line, "dcf sync") == 0)
    {
        if (dcf77_sync_active())
        {
            con_printf("dcf: window already open\n");
        }
        else if (!dcf77_sync_open(board_ms()))
        {
            con_printf("dcf: open failed\n");
        }
        else
        {
            con_printf("dcf: window open\n");
        }
        return;
    }
    clock_time next;
    if (!clock_parse(s_line, &next))
    {
        con_printf("bad time, want YYYY-MM-DD HH:MM:SS\n");
        return;
    }
    s_time = next;
    s_set = true;
    s_last_ms = board_ms();
    s_draw = true;
    con_printf("set %04d-%02d-%02d %02d:%02d:%02d %s\n",
               s_time.year, s_time.month, s_time.day,
               s_time.hour, s_time.min, s_time.sec,
               clock_wday_pl(s_time.wday));
    announce_alarm(alarm_on_clock_set(&s_time));
}

static void poll_uart(void)
{
    char c;
    char echo[24];
    unsigned echo_len = 0;
    /* Echo after the drain. A TX wait in the middle would leave bytes sitting. */
    while (con_getc(&c))
    {
        /* picocom sends CR for Enter. A following LF must not parse an empty line. */
        if (c == '\r' || c == '\n')
        {
            if (echo_len > 0)
            {
                echo[echo_len] = '\0';
                con_printf("%s", echo);
                echo_len = 0;
            }
            if (s_line_len > 0)
            {
                accept_line();
            }
            continue;
        }
        if (s_line_len + 1 < sizeof s_line)
        {
            s_line[s_line_len++] = c;
        }
        else
        {
            s_line_len = 0;
        }
        if (echo_len + 1 < sizeof echo)
        {
            echo[echo_len++] = c;
        }
    }
    if (echo_len > 0)
    {
        echo[echo_len] = '\0';
        con_printf("%s", echo);
    }
}

static void poll_clock(void)
{
    if (!s_set || s_drawing)
    {
        return;
    }
    uint32_t now = board_ms();
    uint32_t elapsed = now - s_last_ms;
    if (elapsed < 1000)
    {
        return;
    }
    uint32_t steps = elapsed / 1000u;
    int prev_min = s_time.min;
    clock_add_seconds(&s_time, steps);
    s_last_ms += steps * 1000u;
    nrf_gpio_pin_toggle(CLOCK_SEC_MARK);
    con_printf("%04d-%02d-%02d %02d:%02d:%02d %s\n",
               s_time.year, s_time.month, s_time.day,
               s_time.hour, s_time.min, s_time.sec,
               clock_wday_pl(s_time.wday));
    if (s_time.min != prev_min || steps >= 60u)
    {
        announce_alarm(alarm_on_minute(&s_time));
    }
    if (s_time.min != prev_min && s_time.hour == 3 && s_time.min == 0)
    {
        s_dcf_retries_left = DCF_RETRIES;
        s_dcf_retry_wait = false;
        s_dcf_pending = true;
        s_dcf_pending_ms = now;
    }
    if (s_time.min != s_drawn_min)
    {
        s_draw = true;
    }
}

static volatile bool s_int1;

static void on_int1(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    (void)pin;
    (void)action;
    s_int1 = true;
}

static void motion_arm(void)
{
    nrfx_gpiote_in_config_t cfg = NRFX_GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    cfg.pull = NRF_GPIO_PIN_PULLDOWN;
    if (nrfx_gpiote_in_init(SENSOR_INT1, &cfg, on_int1) != NRFX_SUCCESS)
    {
        con_printf("motion: INT1 channel busy\n");
        return;
    }
    s_int1 = false;
    motion_count_reset();
    nrfx_gpiote_in_event_enable(SENSOR_INT1, true);
    lis3dh_motion_int1_enable();
}

static void motion_disarm(void)
{
    nrfx_gpiote_in_event_disable(SENSOR_INT1);
    nrfx_gpiote_in_uninit(SENSOR_INT1);
    lis3dh_power_down();
    motion_count_reset();
    s_int1 = false;
}

static void poll_alarm(void)
{
    static bool buzzing;
    static bool was_active;
    bool want = alarm_step(board_ms(), board_any_button_pressed());
    if (alarm_active() && !was_active)
    {
        motion_arm();
    }
    if (alarm_active() && s_int1)
    {
        s_int1 = false;
        uint32_t now = board_ms();
        (void)lis3dh_int1_source(); /* releases the latch */
        if (motion_count_event(now))
        {
            con_printf("MOTION event %u (stop)\n", MOTION_N);
            alarm_stop_motion();
            want = false;
        }
        else
        {
            con_printf("MOTION event %u\n", motion_count_events());
        }
    }
    if (!alarm_active() && was_active)
    {
        motion_disarm();
    }
    was_active = alarm_active();
    if (want != buzzing)
    {
        buzzing = want;
        if (want)
        {
            buzzer_tone_start();
        }
        else
        {
            buzzer_off();
        }
    }
    alarm_stop_reason why = alarm_take_stop();
    if (why != ALARM_STOP_NONE)
    {
        con_printf("ALARM stop (%s)\n", why == ALARM_STOP_BUTTON   ? "button"
                                         : why == ALARM_STOP_MOTION ? "motion"
                                                                    : "timeout");
    }
}

static void print_time(const clock_time *t)
{
    con_printf("%04d-%02d-%02d %02d:%02d:%02d", t->year, t->month, t->day,
               t->hour, t->min, t->sec);
}

static void poll_dcf(void)
{
    uint32_t now = board_ms();
    if (s_dcf_pending)
    {
        if (dcf77_sync_active())
        {
            s_dcf_pending = false;
        }
        else if (!alarm_active())
        {
            s_dcf_pending = false;
            if (!dcf77_sync_open(now))
            {
                con_printf("dcf: open failed\n");
            }
            else
            {
                con_printf("dcf: window open\n");
            }
        }
        else if (now - s_dcf_pending_ms > DCF_LATE_MS)
        {
            s_dcf_pending = false;
            con_printf("dcf: window dropped, alarm active\n");
        }
    }
    if (s_dcf_retry_wait && (int32_t)(now - s_dcf_retry_ms) >= 0)
    {
        s_dcf_retry_wait = false;
        s_dcf_pending = true;
        s_dcf_pending_ms = now;
    }

    dcf77_sync_result_t res;
    dcf77_sync_step(now, &res);
    if (res.outcome == DCF77_SYNC_OK)
    {
        clock_time before = s_time;
        bool was_set = s_set;
        s_time = res.time;
        s_set = true;
        s_last_ms = res.edge_ms;
        s_draw = true;
        s_dcf_retries_left = 0;
        s_dcf_retry_wait = false;
        con_printf("dcf: ok ");
        if (was_set)
        {
            print_time(&before);
        }
        else
        {
            con_printf("unset");
        }
        con_printf(" -> ");
        print_time(&s_time);
        con_printf("\n");
        announce_alarm(alarm_on_clock_set(&s_time));
    }
    else if (res.outcome == DCF77_SYNC_REJECTED)
    {
        if (res.reason == DCF77_REASON_TIMEOUT)
        {
            con_printf("dcf: timeout (time unchanged ");
        }
        else
        {
            con_printf("dcf: rejected %s (time unchanged ", dcf77_reason_name(res.reason));
        }
        if (s_set)
        {
            print_time(&s_time);
        }
        else
        {
            con_printf("unset");
        }
        con_printf(")\n");
    }
    if (res.closed && res.outcome != DCF77_SYNC_OK && s_dcf_retries_left > 0)
    {
        s_dcf_retries_left--;
        s_dcf_retry_wait = true;
        s_dcf_retry_ms = now + DCF_RETRY_MS;
    }
}

static void want_wake(bool *have, uint32_t *next, uint32_t t)
{
    if (!*have || (int32_t)(t - *next) < 0)
    {
        *next = t;
        *have = true;
    }
}

/* Earliest time any handler has work without an interrupt. ISR-driven work
 * (UART, INT1, DCF edges, buttons) wakes the CPU by itself. */
static void arm_wake(void)
{
    uint32_t now = board_ms();
    uint32_t next = now;
    bool have = false;
    if (s_set)
    {
        want_wake(&have, &next, s_last_ms + 1000u);
    }
    if (alarm_active())
    {
        want_wake(&have, &next, alarm_next_ms(now));
    }
    if (dcf77_sync_active())
    {
        want_wake(&have, &next, dcf77_sync_deadline_ms());
    }
    if (s_dcf_pending)
    {
        want_wake(&have, &next, s_dcf_pending_ms + DCF_LATE_MS + 1u);
    }
    if (s_dcf_retry_wait)
    {
        want_wake(&have, &next, s_dcf_retry_ms);
    }
    if (have)
    {
        board_wake_at(BOARD_WAKE_APP, next);
    }
    else
    {
        board_wake_cancel(BOARD_WAKE_APP);
    }
}

void app_poll(void)
{
    poll_uart();
    poll_clock();
    poll_alarm();
    poll_dcf();
    /* Also reached from driver waits (e-paper refresh), so the buzzer keeps its phase. */
    arm_wake();
}

static bool face_pixel(uint32_t x, uint32_t y, void *ctx)
{
    return clock_face_black(x, y, ctx);
}

int main(void)
{
    board_safe_state();
    nrf_gpio_pin_clear(CLOCK_SEC_MARK);
    nrf_gpio_cfg_output(CLOCK_SEC_MARK);
    alarm_init();
    dcf77_sync_init();
    board_button_wake_enable();
    con_init();
    board_time_init();
    spi_bus_init();
    lis3dh_power_down();
    clock_set(&s_time, 2000, 1, 1, 0, 0, 0);

    con_printf("clock-project clock " BUILD_ID "\n");
    con_printf("LFXO RTC1. Send YYYY-MM-DD HH:MM:SS or alarm <1-7> HH:MM on|off, alarm?\n");
    draw();
    s_dcf_retries_left = DCF_RETRIES;
    s_dcf_pending = true;
    s_dcf_pending_ms = board_ms();
    /* Boot marker: low from reset until here, so its first rising edge is a
     * PPK2 digital trigger that skips power-on, LFXO start and the first
     * refresh. Later second ticks toggle from this level. */
    nrf_gpio_pin_set(CLOCK_SEC_MARK);

    for (;;)
    {
        app_poll();
        /* A refresh inside the sync window corrupts pulses; the window is at most 5 min. */
        if (s_draw && !s_drawing && !dcf77_sync_active())
        {
            draw();
            continue;
        }
        board_sleep();
    }
}
