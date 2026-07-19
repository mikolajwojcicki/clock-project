/**
 * SoftDevice-free LED blinky for nRF52 DK (PCA10040).
 *
 * Based on Nordic nRF5 SDK 17.1.0 examples/peripheral/blinky (blank).
 */

#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"

int main(void)
{
    bsp_board_init(BSP_INIT_LEDS);

    while (true)
    {
        for (int i = 0; i < LEDS_NUMBER; i++)
        {
            bsp_board_led_invert(i);
            nrf_delay_ms(500);
        }
    }
}
