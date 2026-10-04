#include "spi_bus.h"

#include "dk_breadboard_pins.h"
#include "nrf.h"
#include "nrf_gpio.h"

void spi_bus_init(void)
{
    nrf_gpio_pin_clear(SPI_SCK);
    nrf_gpio_cfg(SPI_SCK, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_CONNECT,
                 NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_pin_clear(SPI_MOSI);
    nrf_gpio_cfg_output(SPI_MOSI);
    nrf_gpio_cfg_input(SPI_MISO, NRF_GPIO_PIN_NOPULL);

    NRF_SPIM0->PSEL.SCK = SPI_SCK;
    NRF_SPIM0->PSEL.MOSI = SPI_MOSI;
    NRF_SPIM0->PSEL.MISO = SPI_MISO;
    NRF_SPIM0->FREQUENCY = SPIM_FREQUENCY_FREQUENCY_M1;
    NRF_SPIM0->ORC = 0xFF;
}

void spi_bus_transfer(uint32_t cs_pin, uint8_t mode,
                      const uint8_t *tx, size_t tx_len,
                      uint8_t *rx, size_t rx_len)
{
    /* SCK idles at the GPIO level while SPIM0 is disabled: match CPOL first. */
    if (mode == 3)
    {
        nrf_gpio_pin_set(SPI_SCK);
        NRF_SPIM0->CONFIG = (SPIM_CONFIG_CPOL_ActiveLow << SPIM_CONFIG_CPOL_Pos) |
                            (SPIM_CONFIG_CPHA_Trailing << SPIM_CONFIG_CPHA_Pos);
    }
    else
    {
        nrf_gpio_pin_clear(SPI_SCK);
        NRF_SPIM0->CONFIG = 0;
    }
    NRF_SPIM0->ENABLE = SPIM_ENABLE_ENABLE_Enabled << SPIM_ENABLE_ENABLE_Pos;

    NRF_SPIM0->TXD.PTR = (uint32_t)tx;
    NRF_SPIM0->TXD.MAXCNT = tx_len;
    NRF_SPIM0->RXD.PTR = (uint32_t)rx;
    NRF_SPIM0->RXD.MAXCNT = rx_len;

    nrf_gpio_pin_clear(cs_pin);
    NRF_SPIM0->EVENTS_END = 0;
    NRF_SPIM0->TASKS_START = 1;
    while (!NRF_SPIM0->EVENTS_END)
    {
    }
    nrf_gpio_pin_set(cs_pin);

    NRF_SPIM0->ENABLE = 0;
}
