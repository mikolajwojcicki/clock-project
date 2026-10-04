/**
 * Breadboard bring-up firmware for the nRF52 DK (PCA10040).
 *
 * Holds every peripheral in its safe state and runs one test per DK button.
 * Results go to the DK virtual COM port (115200 8N1).
 * Wiring and test procedure: hardware/docs/breadboard-prototype-guide.md.
 */

#include <stdbool.h>
#include <stdint.h>

#include "board.h"
#include "dk_breadboard_pins.h"
#include "nrf.h"
#include "nrf_gpio.h"

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

static volatile uint8_t s_running;
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

static void run_stub(uint8_t t)
{
    board_wait_ms(3000);
    finish(t, "STOPPED", "not implemented yet");
}

static void run_test(uint8_t t)
{
    s_stop = false;
    s_running = t;
    con_printf("\n== %s test ==\n", test_names[t]);
    run_stub(t);
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
