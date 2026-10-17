#include "board.h"
#include "alarm.h"
#include "buzzer.h"
#include "clock.h"
#include "clock_face.h"
#include "epd.h"
#include "spi_bus.h"

#include <stdbool.h>
#include <stdint.h>

static clock_time s_time;
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
    if (!epd_refresh(face_pixel, &s_time))
    {
        con_printf("display FAIL\n");
    }
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
    con_printf("%04d-%02d-%02d %02d:%02d:%02d %s\n",
               s_time.year, s_time.month, s_time.day,
               s_time.hour, s_time.min, s_time.sec,
               clock_wday_pl(s_time.wday));
    if (s_time.min != prev_min || steps >= 60u)
    {
        announce_alarm(alarm_on_minute(&s_time));
    }
    if (s_time.min != s_drawn_min)
    {
        s_draw = true;
    }
}

static void poll_alarm(void)
{
    static bool buzzing;
    bool want = alarm_step(board_ms(), board_any_button_pressed());
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
        con_printf("ALARM stop (%s)\n", why == ALARM_STOP_BUTTON ? "button" : "timeout");
    }
}

void app_poll(void)
{
    poll_uart();
    poll_clock();
    poll_alarm();
}

static bool face_pixel(uint32_t x, uint32_t y, void *ctx)
{
    return clock_face_black(x, y, ctx);
}

int main(void)
{
    board_safe_state();
    alarm_init();
    con_init();
    board_time_init();
    spi_bus_init();
    clock_set(&s_time, 2000, 1, 1, 0, 0, 0);

    con_printf("clock-project clock " BUILD_ID "\n");
    con_printf("LFXO RTC1. Send YYYY-MM-DD HH:MM:SS or alarm <1-7> HH:MM on|off, alarm?\n");
    draw();

    for (;;)
    {
        app_poll();
        if (s_draw && !s_drawing)
        {
            draw();
        }
    }
}
