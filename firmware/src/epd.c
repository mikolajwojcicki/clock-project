/* Waveshare Arduino epd2in13_V4 Init(FULL) + DisplayPartBaseImage, then the
 * GxEPD2 post-refresh "write again" step so the SSD1680 buffer swap does not
 * turn the next minute upside down.
 * https://github.com/waveshareteam/e-Paper (2023-06-25)
 * https://github.com/ZinggJM/GxEPD2 GxEPD2_213_B74 */
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

static bool test_pixel(uint32_t x, uint32_t y, void *ctx)
{
    (void)ctx;
    return is_black(x, y);
}

/* Same as Waveshare SetWindows + SetCursor for the full panel. */
static void set_ram_area(void)
{
    cmd(0x11); /* data entry: X and Y increment */
    data(0x03);
    cmd(0x44);
    data(0x00);
    data((EPD_WIDTH - 1) >> 3);
    cmd(0x45);
    data(0x00);
    data(0x00);
    data((EPD_HEIGHT - 1) & 0xFF);
    data((EPD_HEIGHT - 1) >> 8);
    cmd(0x4E);
    data(0x00);
    cmd(0x4F);
    data(0x00);
    data(0x00);
}

static void write_face(uint8_t ram_cmd, epd_pixel_fn pixel, void *ctx)
{
    set_ram_area();
    cmd(ram_cmd); /* bit 1 = white */
    for (uint32_t y = 0; y < EPD_HEIGHT; y++)
    {
        uint8_t row[EPD_ROW_BYTES];
        for (uint32_t b = 0; b < EPD_ROW_BYTES; b++)
        {
            uint8_t v = 0xFF;
            for (uint32_t bit = 0; bit < 8; bit++)
            {
                uint32_t x = b * 8 + bit;
                if (x < EPD_WIDTH && pixel(x, y, ctx))
                {
                    v &= (uint8_t)~(0x80 >> bit);
                }
            }
            row[b] = v;
        }
        send(true, row, sizeof row);
        app_poll(); /* full-panel write takes seconds; keep buzzer pulsing */
    }
}

static void write_both_rams(epd_pixel_fn pixel, void *ctx)
{
    /* Waveshare DisplayPartBaseImage order: BW then RED/previous. */
    write_face(0x24, pixel, ctx);
    write_face(0x26, pixel, ctx);
}

bool epd_show_test_pattern(void)
{
    return epd_refresh(test_pixel, NULL);
}

bool epd_refresh(epd_pixel_fn pixel, void *ctx)
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
    data(0x00); /* official driver: GD=0, SM=0, TB=0 */
    set_ram_area();
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

    write_both_rams(pixel, ctx);

    cmd(0x22); /* full update sequence */
    data(0xF7);
    cmd(0x20); /* master activation */
    if (!wait_idle())
    {
        return false;
    }

    /* Full update swaps the two RAMs. Without this, the next minute comes from
     * the other buffer and the glass reads upside down (2→5, Wt→M). GxEPD2
     * does the same as writeImageAgain after refresh. */
    write_both_rams(pixel, ctx);

    cmd(0x10); /* deep sleep mode 1; needs a hardware reset to wake */
    data(0x01);
    board_wait_ms(100);
    return true;
}
