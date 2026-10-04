/* Command sequence follows Waveshare's EPD_2in13_V4 reference driver. */
#include "epd.h"

#include <stdint.h>

#include "board.h"
#include "dk_breadboard_pins.h"
#include "nrf_gpio.h"
#include "spi_bus.h"

#define EPD_WIDTH      122
#define EPD_HEIGHT     250
#define EPD_ROW_BYTES  ((EPD_WIDTH + 7) / 8)
#define EPD_BUSY_TIMEOUT_MS 10000
#define BORDER_PX      4
#define CHECKER_PX     16

static void send(bool is_data, const uint8_t *buf, size_t len)
{
    if (is_data)
    {
        nrf_gpio_pin_set(EPD_DC);
    }
    else
    {
        nrf_gpio_pin_clear(EPD_DC);
    }
    spi_bus_transfer(EPD_CS, 0, buf, len, NULL, 0);
}

static void cmd(uint8_t c)
{
    send(false, &c, 1);
}

static void data(uint8_t d)
{
    send(true, &d, 1);
}

/* BUSY is high while the controller works. */
static bool wait_idle(void)
{
    uint32_t start = board_ms();
    while (nrf_gpio_pin_read(EPD_BUSY))
    {
        if (board_ms() - start > EPD_BUSY_TIMEOUT_MS)
        {
            return false;
        }
        app_poll();
    }
    return true;
}

static bool is_black(uint32_t x, uint32_t y)
{
    if (x < BORDER_PX || x >= EPD_WIDTH - BORDER_PX ||
        y < BORDER_PX || y >= EPD_HEIGHT - BORDER_PX)
    {
        return true;
    }
    return ((x / CHECKER_PX) + (y / CHECKER_PX)) & 1;
}

bool epd_show_test_pattern(void)
{
    nrf_gpio_pin_set(EPD_RST);
    nrf_gpio_cfg_output(EPD_RST);
    nrf_gpio_cfg_output(EPD_DC);
    /* Pull-up: a missing BUSY wire reads busy and fails the test, not passes it. */
    nrf_gpio_cfg_input(EPD_BUSY, NRF_GPIO_PIN_PULLUP);

    board_wait_ms(20);
    nrf_gpio_pin_clear(EPD_RST);
    board_wait_ms(2);
    nrf_gpio_pin_set(EPD_RST);
    board_wait_ms(20);
    if (!wait_idle())
    {
        return false;
    }

    cmd(0x12); /* software reset */
    if (!wait_idle())
    {
        return false;
    }

    cmd(0x01); /* driver output control: 250 gate lines */
    data((EPD_HEIGHT - 1) & 0xFF);
    data((EPD_HEIGHT - 1) >> 8);
    data(0x00);
    cmd(0x11); /* data entry: X and Y increment */
    data(0x03);
    cmd(0x44); /* RAM X window, in bytes */
    data(0x00);
    data((EPD_WIDTH - 1) >> 3);
    cmd(0x45); /* RAM Y window */
    data(0x00);
    data(0x00);
    data((EPD_HEIGHT - 1) & 0xFF);
    data((EPD_HEIGHT - 1) >> 8);
    cmd(0x4E); /* RAM X counter */
    data(0x00);
    cmd(0x4F); /* RAM Y counter */
    data(0x00);
    data(0x00);
    cmd(0x3C); /* border waveform */
    data(0x05);
    cmd(0x21); /* display update control */
    data(0x00);
    data(0x80);
    cmd(0x18); /* internal temperature sensor */
    data(0x80);
    if (!wait_idle())
    {
        return false;
    }

    cmd(0x24); /* write black/white RAM; bit 1 = white */
    for (uint32_t y = 0; y < EPD_HEIGHT; y++)
    {
        uint8_t row[EPD_ROW_BYTES];
        for (uint32_t b = 0; b < EPD_ROW_BYTES; b++)
        {
            uint8_t v = 0xFF;
            for (uint32_t bit = 0; bit < 8; bit++)
            {
                uint32_t x = b * 8 + bit;
                if (x < EPD_WIDTH && is_black(x, y))
                {
                    v &= (uint8_t)~(0x80 >> bit);
                }
            }
            row[b] = v;
        }
        send(true, row, sizeof row);
    }

    cmd(0x22); /* full update sequence */
    data(0xF7);
    cmd(0x20); /* master activation */
    if (!wait_idle())
    {
        return false;
    }

    cmd(0x10); /* deep sleep mode 1; needs a hardware reset to wake */
    data(0x01);
    board_wait_ms(100);
    return true;
}
