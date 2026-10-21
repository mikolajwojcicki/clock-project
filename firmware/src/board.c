#include "board.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#include "buzzer.h"
#include "dk_breadboard_pins.h"
#include "nrf.h"
#include "nrf_gpio.h"
#include "nrfx_clock.h"
#include "nrfx_gpiote.h"
#include "nrfx_rtc.h"

#include "app_uart.h"
#include "nrf_uart.h"

void board_safe_state(void)
{
    /* Set OUT before DIR so the pins never glitch to the active level. */
    nrf_gpio_pin_set(DCF_PON);
    nrf_gpio_cfg_output(DCF_PON);
    nrf_gpio_pin_set(EPD_CS);
    nrf_gpio_cfg_output(EPD_CS);
    nrf_gpio_pin_set(SENSOR_CS);
    nrf_gpio_cfg_output(SENSOR_CS);
    nrf_gpio_pin_clear(BUZZER_EN);
    nrf_gpio_cfg_output(BUZZER_EN);
    buzzer_off();
    nrf_gpio_cfg_input(DK_BUTTON1, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(DK_BUTTON2, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(DK_BUTTON3, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(DK_BUTTON4, NRF_GPIO_PIN_PULLUP);
}

bool board_any_button_pressed(void)
{
    return !nrf_gpio_pin_read(DK_BUTTON1) || !nrf_gpio_pin_read(DK_BUTTON2) ||
           !nrf_gpio_pin_read(DK_BUTTON3) || !nrf_gpio_pin_read(DK_BUTTON4);
}

static void on_button(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    /* The interrupt is the wake; app_poll reads the pin levels. */
    (void)pin;
    (void)action;
}

void board_button_wake_enable(void)
{
    static const uint32_t pins[] = { DK_BUTTON1, DK_BUTTON2, DK_BUTTON3, DK_BUTTON4 };
    /* hi_accuracy false: PORT event via SENSE, no IN channel, no extra idle current.
     * A held button keeps DETECT high and masks other SENSE pins, so only buttons use it. */
    nrfx_gpiote_in_config_t cfg = NRFX_GPIOTE_CONFIG_IN_SENSE_HITOLO(false);
    cfg.pull = NRF_GPIO_PIN_PULLUP;
    for (unsigned i = 0; i < sizeof pins / sizeof pins[0]; i++)
    {
        if (nrfx_gpiote_in_init(pins[i], &cfg, on_button) == NRFX_SUCCESS)
        {
            nrfx_gpiote_in_event_enable(pins[i], true);
        }
    }
}

static volatile uint32_t s_rtc_overflows;
static const nrfx_rtc_t s_rtc = NRFX_RTC_INSTANCE(1);

static void clock_evt(nrfx_clock_evt_type_t evt)
{
    (void)evt;
}

static void rtc_evt(nrfx_rtc_int_type_t type)
{
    /* Compare channels: nrfx already disabled the channel (one-shot); the IRQ is the wake. */
    if (type == NRFX_RTC_INT_OVERFLOW)
    {
        s_rtc_overflows++;
    }
}

void board_time_init(void)
{
    nrfx_clock_init(clock_evt);
    nrfx_clock_enable();
    nrfx_clock_lfclk_start();
    /* LFXO starts in ~0.3 s. Bounded so a missing crystal cannot hang boot;
     * the RTC then runs from the RC fallback. */
    for (uint32_t i = 0; i < 8000000 && !nrfx_clock_lfclk_is_running(); i++)
    {
    }

    const nrfx_rtc_config_t cfg = {
        .prescaler          = 0,
        .interrupt_priority = NRFX_RTC_DEFAULT_CONFIG_IRQ_PRIORITY,
        .reliable           = false,
        .tick_latency       = NRFX_RTC_US_TO_TICKS(2000, 32768),
    };
    nrfx_rtc_init(&s_rtc, &cfg, rtc_evt);
    nrfx_rtc_overflow_enable(&s_rtc, true);
    nrfx_rtc_enable(&s_rtc);
}

uint32_t board_ms(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    uint32_t counter = nrfx_rtc_counter_get(&s_rtc);
    uint32_t overflows = s_rtc_overflows;
    if (nrf_rtc_event_pending(s_rtc.p_reg, NRF_RTC_EVENT_OVERFLOW))
    {
        /* Overflow happened but its IRQ has not run yet. */
        counter = nrfx_rtc_counter_get(&s_rtc);
        overflows++;
    }
    __set_PRIMASK(primask);

    uint64_t ticks = ((uint64_t)overflows << 24) | counter;
    return (uint32_t)((ticks * 1000u) >> 15);
}

__attribute__((weak)) void app_poll(void)
{
}

/* Set by board_wake_at for a target already reached; thread context only. */
static bool s_wake_due;

void board_wake_at(board_wake_t ch, uint32_t target_ms)
{
    int32_t d = (int32_t)(target_ms - board_ms());
    if (d <= 0)
    {
        (void)nrfx_rtc_cc_disable(&s_rtc, ch);
        s_wake_due = true;
        return;
    }
    /* The counter is 24 bits (512 s); a far target wakes early and the owner re-arms. */
    if (d > 500000)
    {
        d = 500000;
    }
    /* Round up: board_ms() floors ticks, so the wake lands at or after target_ms. */
    uint32_t ticks = (uint32_t)(((uint64_t)d * 32768u + 999u) / 1000u);
    uint32_t cc = (nrfx_rtc_counter_get(&s_rtc) + ticks) & 0xFFFFFFu;
    (void)nrfx_rtc_cc_set(&s_rtc, ch, cc, true);
}

void board_wake_cancel(board_wake_t ch)
{
    (void)nrfx_rtc_cc_disable(&s_rtc, ch);
}

void board_sleep(void)
{
    if (s_wake_due)
    {
        s_wake_due = false;
        return;
    }
#if (__FPU_USED == 1)
    /* nRF52 anomaly 87: a set FPU exception flag keeps FPU_IRQn pending and blocks sleep. */
    __set_FPSCR(__get_FPSCR() & ~0x0000009Fu);
    (void)__get_FPSCR();
    NVIC_ClearPendingIRQ(FPU_IRQn);
#endif
    /* An IRQ taken since the caller checked its flags set the event register,
     * so the first WFE returns at once instead of losing that wake. SEV+WFE clears it. */
    __WFE();
    __SEV();
    __WFE();
}

void board_wait_ms(uint32_t ms)
{
    uint32_t end = board_ms() + ms;
    for (;;)
    {
        app_poll();
        if ((int32_t)(board_ms() - end) >= 0)
        {
            break;
        }
        board_wake_at(BOARD_WAKE_WAIT, end);
        board_sleep();
    }
    board_wake_cancel(BOARD_WAKE_WAIT);
}

void app_error_handler_bare(uint32_t error_code)
{
    (void)error_code;
}

static bool s_con_ok;

static void uart_evt(app_uart_evt_t *event)
{
    (void)event;
}

void con_init(void)
{
    uint32_t err;
    /* PCA10040 uart example: EasyDMA, flow control off, then echo bytes. */
    const app_uart_comm_params_t comm = {
        DK_UART_RX,
        DK_UART_TX,
        DK_UART_RTS,
        DK_UART_CTS,
        APP_UART_FLOW_CONTROL_DISABLED,
        false,
        NRF_UART_BAUDRATE_115200
    };

    APP_UART_FIFO_INIT(&comm, 256, 256, uart_evt, APP_IRQ_PRIORITY_LOWEST, err);
    if (err != NRF_SUCCESS)
    {
        /* No console. The clock loop still runs. */
        return;
    }
    s_con_ok = true;
}

bool con_getc(char *out)
{
    uint8_t byte;
    if (app_uart_get(&byte) != NRF_SUCCESS)
    {
        return false;
    }
    *out = (char)byte;
    return true;
}

static void con_putc(char c)
{
    if (!s_con_ok)
    {
        return;
    }
    /* Bounded: a stuck TX must not hang the clock loop. */
    for (uint32_t i = 0; i < 200000 && app_uart_put((uint8_t)c) != NRF_SUCCESS; i++)
    {
    }
}

void con_printf(const char *fmt, ...)
{
    char buf[160];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);

    for (const char *p = buf; *p; p++)
    {
        if (*p == '\n')
        {
            con_putc('\r');
        }
        con_putc(*p);
    }
}
