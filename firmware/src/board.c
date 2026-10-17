#include "board.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#include "buzzer.h"
#include "dk_breadboard_pins.h"
#include "nrf.h"
#include "nrf_gpio.h"
#include "nrfx_clock.h"
#include "nrfx_rtc.h"

#ifdef CLOCK_APP
#include "app_uart.h"
#include "nrf_uart.h"
#endif

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

static volatile uint32_t s_rtc_overflows;
static const nrfx_rtc_t s_rtc = NRFX_RTC_INSTANCE(1);

static void clock_evt(nrfx_clock_evt_type_t evt)
{
    (void)evt;
}

static void rtc_evt(nrfx_rtc_int_type_t type)
{
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
    while (!nrfx_clock_lfclk_is_running())
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

void board_wait_ms(uint32_t ms)
{
    uint32_t start = board_ms();
    while (board_ms() - start < ms)
    {
        app_poll();
    }
}

#ifdef CLOCK_APP
void app_error_handler_bare(uint32_t error_code)
{
    (void)error_code;
}

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
    while (app_uart_put((uint8_t)c) != NRF_SUCCESS)
    {
    }
}
#else
void con_init(void)
{
    nrf_gpio_pin_set(DK_UART_TX);
    nrf_gpio_cfg_output(DK_UART_TX);
    nrf_gpio_cfg_input(DK_UART_RX, NRF_GPIO_PIN_NOPULL);
    /* Hold RTS low ourselves. The UART's own RTS stayed high, so the
     * DK interface MCU kept every keystroke. CTS is unused. */
    nrf_gpio_pin_clear(DK_UART_RTS);
    nrf_gpio_cfg_output(DK_UART_RTS);

    NRF_UART0->CONFIG = UART_CONFIG_HWFC_Disabled << UART_CONFIG_HWFC_Pos;
    NRF_UART0->PSELTXD = DK_UART_TX;
    NRF_UART0->PSELRXD = DK_UART_RX;
    NRF_UART0->PSELRTS = 0xFFFFFFFFu;
    NRF_UART0->PSELCTS = 0xFFFFFFFFu;
    NRF_UART0->BAUDRATE = UART_BAUDRATE_BAUDRATE_Baud115200;
    NRF_UART0->ENABLE = UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos;
    NRF_UART0->EVENTS_RXDRDY = 0;
    NRF_UART0->EVENTS_ERROR = 0;
    NRF_UART0->ERRORSRC = NRF_UART0->ERRORSRC;
    NRF_UART0->TASKS_STARTTX = 1;
    NRF_UART0->TASKS_STARTRX = 1;
}

bool con_getc(char *out)
{
    /* Same order as nrfx_uart: clear the event, then read RXD.
     * Leaving RXDRDY set makes every later read return the same byte. */
    /* A break or overrun stops this UART until STARTRX is written again. */
    if (NRF_UART0->EVENTS_ERROR || NRF_UART0->EVENTS_RXTO)
    {
        NRF_UART0->EVENTS_ERROR = 0;
        (void)NRF_UART0->EVENTS_ERROR;
        NRF_UART0->EVENTS_RXTO = 0;
        (void)NRF_UART0->EVENTS_RXTO;
        NRF_UART0->ERRORSRC = NRF_UART0->ERRORSRC;
        NRF_UART0->TASKS_STARTRX = 1;
    }
    if (!NRF_UART0->EVENTS_RXDRDY)
    {
        return false;
    }
    NRF_UART0->EVENTS_RXDRDY = 0;
    (void)NRF_UART0->EVENTS_RXDRDY;
    *out = (char)NRF_UART0->RXD;
    return true;
}

static void con_putc(char c)
{
    NRF_UART0->EVENTS_TXDRDY = 0;
    NRF_UART0->TXD = (uint8_t)c;
    while (!NRF_UART0->EVENTS_TXDRDY)
    {
    }
}
#endif

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
