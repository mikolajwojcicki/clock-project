/**
 * Breadboard bring-up firmware for the nRF52 DK (PCA10040).
 *
 * Holds every peripheral in its safe state and runs one test per DK button.
 * Results go to the DK virtual COM port (115200 8N1).
 * Wiring and test procedure: hardware/docs/breadboard-prototype-guide.md.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "board.h"
#include "buzzer.h"
#include "dcf77_classify.h"
#include "dk_breadboard_pins.h"
#include "epd.h"
#include "lis3dh.h"
#include "nrf.h"
#include "nrf_gpio.h"
#include "spi_bus.h"

/* Test number == DK button number. */
enum test
{
    TEST_NONE,
    TEST_DISPLAY,
    TEST_LIS3DH,
    TEST_DCF77,
    TEST_BUZZER,
};

static const char *const test_names[] = {"none", "display", "lis3dh", "dcf77", "buzzer"};
static const uint32_t button_pins[] = {DK_BUTTON1, DK_BUTTON2, DK_BUTTON3, DK_BUTTON4};

#define BUTTON_DEBOUNCE_MS 50
#define LIS3DH_TEST_MS     60000u
#define DCF77_TEST_MS      (10u * 60u * 1000u)
#define DCF77_HEARTBEAT_MS 10000u
#define BUZZER_STEP_MS     500u
#define BUZZER_GAP_MS      300u

/* GPIOTE channels 0 to 3 are buttons 1 to 4. */
#define GPIOTE_CH_INT1 4
#define GPIOTE_CH_DCF  5

static volatile uint8_t s_running;
static volatile bool s_int1;

/* ponytail: one pending DCF pulse, no queue. A second pulse before the main
 * loop prints the first counts as an overrun; add a ring buffer if that shows up. */
static volatile bool s_dcf_pulse;
static volatile uint32_t s_dcf_width_ms;
static volatile uint32_t s_dcf_period_ms;
static volatile uint32_t s_dcf_overruns;
static uint32_t s_dcf_start_ms;
static bool s_dcf_have_start;
static volatile uint8_t s_pressed;
static volatile uint8_t s_ignored;
static volatile bool s_stop;
static uint32_t s_last_edge_ms[4];

static bool is_continuous(uint8_t t)
{
    return t == TEST_LIS3DH || t == TEST_DCF77;
}

static void gpiote_in(uint32_t ch, uint32_t pin, uint32_t polarity)
{
    NRF_GPIOTE->CONFIG[ch] = (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos) |
                             (pin << GPIOTE_CONFIG_PSEL_Pos) |
                             (polarity << GPIOTE_CONFIG_POLARITY_Pos);
    NRF_GPIOTE->EVENTS_IN[ch] = 0;
    NRF_GPIOTE->INTENSET = 1u << ch;
}

static void buttons_init(void)
{
    for (uint32_t i = 0; i < 4; i++)
    {
        nrf_gpio_cfg_input(button_pins[i], NRF_GPIO_PIN_PULLUP);
        gpiote_in(i, button_pins[i], GPIOTE_CONFIG_POLARITY_Toggle);
    }
    NVIC_EnableIRQ(GPIOTE_IRQn);
}

static void on_button_edge(uint32_t i)
{
    uint32_t now = board_ms();
    bool quiet = now - s_last_edge_ms[i] >= BUTTON_DEBOUNCE_MS;
    s_last_edge_ms[i] = now;
    if (!quiet || nrf_gpio_pin_read(button_pins[i]))
    {
        return; /* bounce or release */
    }

    uint8_t t = (uint8_t)(i + 1);
    if (s_running == TEST_NONE)
    {
        s_pressed |= 1u << t;
    }
    else if (t == s_running && is_continuous(t))
    {
        s_stop = true;
    }
    else
    {
        s_ignored |= 1u << t;
    }
}

static void on_dcf_edge(void)
{
    uint32_t now = board_ms();
    if (nrf_gpio_pin_read(DCF_OUT) == DCF_OUT_ACTIVE_LEVEL)
    {
        s_dcf_period_ms = s_dcf_have_start ? now - s_dcf_start_ms : 0;
        s_dcf_start_ms = now;
        s_dcf_have_start = true;
    }
    else if (s_dcf_have_start)
    {
        if (s_dcf_pulse)
        {
            s_dcf_overruns++;
            return;
        }
        s_dcf_width_ms = now - s_dcf_start_ms;
        s_dcf_pulse = true;
    }
}

static void gpiote_off(uint32_t ch)
{
    NRF_GPIOTE->INTENCLR = 1u << ch;
    NRF_GPIOTE->CONFIG[ch] = 0;
    NRF_GPIOTE->EVENTS_IN[ch] = 0;
}

void GPIOTE_IRQHandler(void)
{
    for (uint32_t ch = 0; ch < 4; ch++)
    {
        if (NRF_GPIOTE->EVENTS_IN[ch])
        {
            NRF_GPIOTE->EVENTS_IN[ch] = 0;
            on_button_edge(ch);
        }
    }
    if (NRF_GPIOTE->EVENTS_IN[GPIOTE_CH_INT1])
    {
        NRF_GPIOTE->EVENTS_IN[GPIOTE_CH_INT1] = 0;
        s_int1 = true;
    }
    if (NRF_GPIOTE->EVENTS_IN[GPIOTE_CH_DCF])
    {
        NRF_GPIOTE->EVENTS_IN[GPIOTE_CH_DCF] = 0;
        on_dcf_edge();
    }
}

void app_poll(void)
{
    __disable_irq();
    uint8_t ignored = s_ignored;
    s_ignored = 0;
    __enable_irq();

    for (uint8_t t = TEST_DISPLAY; t <= TEST_BUZZER; t++)
    {
        if (ignored & (1u << t))
        {
            con_printf("button %u ignored: %s test already running\n", t, test_names[s_running]);
        }
    }
}

static uint8_t take_press(void)
{
    __disable_irq();
    uint8_t pressed = s_pressed;
    s_pressed = 0;
    __enable_irq();

    for (uint8_t t = TEST_DISPLAY; t <= TEST_BUZZER; t++)
    {
        if (pressed & (1u << t))
        {
            return t;
        }
    }
    return TEST_NONE;
}

static void finish(uint8_t t, const char *verdict, const char *reason)
{
    board_safe_state();
    con_printf("%s %s: %s\n", verdict, test_names[t], reason);
}

static void run_display(void)
{
    con_printf("reset + full refresh, about 3 s; expect black border + checkerboard\n");
    if (epd_show_test_pattern())
    {
        finish(TEST_DISPLAY, "PASS", "refresh done, panel in deep sleep; check the pattern");
    }
    else
    {
        finish(TEST_DISPLAY, "FAIL", "EPD_BUSY did not clear within 10 s");
    }
}

static void run_lis3dh(void)
{
    uint8_t id = lis3dh_who_am_i();
    con_printf("WHO_AM_I = 0x%02x (expect 0x%02x)\n", id, LIS3DH_WHO_AM_I_VALUE);
    char reason[64];
    if (id != LIS3DH_WHO_AM_I_VALUE)
    {
        snprintf(reason, sizeof reason, "WHO_AM_I mismatch 0x%02x; check SCK, MOSI, MISO, CS, VIN", id);
        finish(TEST_LIS3DH, "FAIL", reason);
        return;
    }
    con_printf("identification passed; move the board (button 2 or 60 s stops)\n");

    /* Pull-down: a missing INT1 wire gives no events instead of noise. */
    nrf_gpio_cfg_input(SENSOR_INT1, NRF_GPIO_PIN_PULLDOWN);
    s_int1 = false;
    gpiote_in(GPIOTE_CH_INT1, SENSOR_INT1, GPIOTE_CONFIG_POLARITY_LoToHi);
    lis3dh_motion_int1_enable();

    uint32_t events = 0;
    uint32_t start = board_ms();
    while (!s_stop && board_ms() - start < LIS3DH_TEST_MS)
    {
        app_poll();
        /* The line is level-latched: also catch a rise that happened before the channel was armed. */
        if (s_int1 || nrf_gpio_pin_read(SENSOR_INT1))
        {
            s_int1 = false;
            uint8_t src = lis3dh_int1_source();
            events++;
            con_printf("INT1 event %lu: src=0x%02x%s%s%s\n", (unsigned long)events, src,
                       (src & LIS3DH_INT_XH) ? " X" : "",
                       (src & LIS3DH_INT_YH) ? " Y" : "",
                       (src & LIS3DH_INT_ZH) ? " Z" : "");
        }
    }

    gpiote_off(GPIOTE_CH_INT1);
    lis3dh_power_down();

    snprintf(reason, sizeof reason, "%lu INT1 events (%s)", (unsigned long)events,
             s_stop ? "button 2" : "timeout");
    finish(TEST_LIS3DH, events ? "PASS" : "STOPPED", reason);
}

static void run_dcf77(void)
{
    static const char *const kinds[] = {"0", "1", "invalid"};

    con_printf("connect OUT to P0.25 only after its level was measured;\n"
               "with OUT unconnected, invalid pulses or none are expected\n");
    nrf_gpio_cfg_input(DCF_OUT, DCF_OUT_PULL);
    s_dcf_pulse = false;
    s_dcf_have_start = false;
    s_dcf_overruns = 0;
    gpiote_in(GPIOTE_CH_DCF, DCF_OUT, GPIOTE_CONFIG_POLARITY_Toggle);

    nrf_gpio_pin_clear(DCF_PON);
    con_printf("receiver enabled (PON low); button 3 or 10 min stops\n");

    uint32_t valid = 0, invalid = 0, markers = 0;
    uint32_t start = board_ms();
    uint32_t heartbeat = start;
    while (!s_stop && board_ms() - start < DCF77_TEST_MS)
    {
        app_poll();
        if (s_dcf_pulse)
        {
            __disable_irq();
            uint32_t width = s_dcf_width_ms;
            uint32_t period = s_dcf_period_ms;
            s_dcf_pulse = false;
            __enable_irq();

            if (dcf77_is_minute_gap(period))
            {
                markers++;
                con_printf("-- minute marker (period %lu ms) --\n", (unsigned long)period);
            }
            dcf77_pulse_t kind = dcf77_classify_width(width);
            if (kind == DCF77_INVALID)
            {
                invalid++;
            }
            else
            {
                valid++;
            }
            con_printf("pulse width=%lu ms period=%lu ms -> %s\n",
                       (unsigned long)width, (unsigned long)period, kinds[kind]);
        }
        if (board_ms() - heartbeat >= DCF77_HEARTBEAT_MS)
        {
            heartbeat += DCF77_HEARTBEAT_MS;
            con_printf("t=%lus valid=%lu invalid=%lu\n", (unsigned long)((heartbeat - start) / 1000),
                       (unsigned long)valid, (unsigned long)invalid);
        }
    }

    gpiote_off(GPIOTE_CH_DCF);

    char reason[112];
    snprintf(reason, sizeof reason,
             "valid=%lu invalid=%lu minute_markers=%lu overruns=%lu (%s); receiver off",
             (unsigned long)valid, (unsigned long)invalid, (unsigned long)markers,
             (unsigned long)s_dcf_overruns, s_stop ? "button 3" : "timeout");
    finish(TEST_DCF77, "STOPPED", reason);
}

static void run_buzzer(void)
{
    con_printf("step 1/2: BUZZER_EN steady high for %u ms\n", BUZZER_STEP_MS);
    buzzer_steady_on();
    board_wait_ms(BUZZER_STEP_MS);
    buzzer_off();
    board_wait_ms(BUZZER_GAP_MS);

    con_printf("step 2/2: 2.7 kHz tone for %u ms\n", BUZZER_STEP_MS);
    buzzer_tone_start();
    board_wait_ms(BUZZER_STEP_MS);
    buzzer_off();

    finish(TEST_BUZZER, "PASS", "both steps ran, BUZZER_EN low; record which step sounded");
}

static void run_test(uint8_t t)
{
    s_stop = false;
    s_running = t;
    con_printf("\n== %s test ==\n", test_names[t]);
    switch (t)
    {
        case TEST_DISPLAY:
            run_display();
            break;
        case TEST_LIS3DH:
            run_lis3dh();
            break;
        case TEST_DCF77:
            run_dcf77();
            break;
        case TEST_BUZZER:
            run_buzzer();
            break;
    }
    s_running = TEST_NONE;
}

static void print_banner(uint32_t resetreas)
{
    con_printf("\nclock-project bringup %s (built %s %s)\n", BUILD_ID, __DATE__, __TIME__);
    con_printf("reset reason: 0x%08lx%s\n", (unsigned long)resetreas,
               resetreas ? "" : " (power-on or brown-out)");
    con_printf("buttons: 1=display 2=lis3dh 3=dcf77 4=buzzer (press 2/3 again to stop)\n");
}

int main(void)
{
    board_safe_state();
    board_time_init();
    con_init();

    uint32_t resetreas = NRF_POWER->RESETREAS;
    NRF_POWER->RESETREAS = resetreas;
    print_banner(resetreas);

    spi_bus_init();
    buttons_init();

    for (;;)
    {
        app_poll();
        uint8_t t = take_press();
        if (t == TEST_NONE)
        {
            __WFE();
            continue;
        }
        run_test(t);
    }
}
