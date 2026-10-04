#include "board.h"

#include <stdarg.h>
#include <stdio.h>

#include "dk_breadboard_pins.h"
#include "nrf.h"
#include "nrf_gpio.h"

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
}

static volatile uint32_t s_rtc_overflows;

void RTC1_IRQHandler(void)
{
    if (NRF_RTC1->EVENTS_OVRFLW)
    {
        NRF_RTC1->EVENTS_OVRFLW = 0;
        (void)NRF_RTC1->EVENTS_OVRFLW;
        s_rtc_overflows++;
    }
}

void board_time_init(void)
{
    NRF_CLOCK->LFCLKSRC = CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos;
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_LFCLKSTART = 1;
    while (!NRF_CLOCK->EVENTS_LFCLKSTARTED)
    {
    }

    NRF_RTC1->PRESCALER = 0;
    NRF_RTC1->EVTENSET = RTC_EVTEN_OVRFLW_Msk;
    NRF_RTC1->INTENSET = RTC_INTENSET_OVRFLW_Msk;
    NVIC_EnableIRQ(RTC1_IRQn);
    NRF_RTC1->TASKS_START = 1;
}

uint32_t board_ms(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    uint32_t counter = NRF_RTC1->COUNTER;
    uint32_t overflows = s_rtc_overflows;
    if (NRF_RTC1->EVENTS_OVRFLW)
    {
        /* Overflow happened but its IRQ has not run yet. */
        counter = NRF_RTC1->COUNTER;
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

void con_init(void)
{
    nrf_gpio_pin_set(DK_UART_TX);
    nrf_gpio_cfg_output(DK_UART_TX);
    nrf_gpio_cfg_input(DK_UART_RX, NRF_GPIO_PIN_PULLUP);

    NRF_UART0->PSELTXD = DK_UART_TX;
    NRF_UART0->PSELRXD = DK_UART_RX;
    NRF_UART0->BAUDRATE = UART_BAUDRATE_BAUDRATE_Baud115200;
    NRF_UART0->ENABLE = UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos;
    NRF_UART0->TASKS_STARTTX = 1;
}

static void con_putc(char c)
{
    NRF_UART0->EVENTS_TXDRDY = 0;
    NRF_UART0->TXD = (uint8_t)c;
    while (!NRF_UART0->EVENTS_TXDRDY)
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
