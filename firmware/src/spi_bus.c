#include "spi_bus.h"

#include "dk_breadboard_pins.h"
#include "nrf_gpio.h"
#include "nrfx_spim.h"

static const nrfx_spim_t s_spim = NRFX_SPIM_INSTANCE(0);

/* Run C4 (2026-10-25): SCK and MOSI held low between transfers sank about
 * 0.57 mA each into the LIS3DH breakout pull-ups (Adafruit 2809). Idle them
 * high instead: never undefined, and no current with the peripherals powered. */
static void bus_idle_high(void)
{
    nrf_gpio_pin_set(SPI_SCK);
    nrf_gpio_cfg_output(SPI_SCK);
    nrf_gpio_pin_set(SPI_MOSI);
    nrf_gpio_cfg_output(SPI_MOSI);
}

void spi_bus_init(void)
{
    bus_idle_high();
    nrf_gpio_cfg_input(SPI_MISO, NRF_GPIO_PIN_NOPULL);
}

void spi_bus_transfer(uint32_t cs_pin, uint8_t mode,
                      const uint8_t *tx, size_t tx_len,
                      uint8_t *rx, size_t rx_len)
{
    /* The SPIM is enabled only for one transfer. nrfx_spim_init drives SCK to
     * the idle level of this mode while every CS is still high, so the mode 0
     * falling edge clocks no device. CS is a plain GPIO: two devices with
     * different CS pins share the bus. */
    const nrfx_spim_config_t cfg = {
        .sck_pin        = SPI_SCK,
        .mosi_pin       = SPI_MOSI,
        .miso_pin       = SPI_MISO,
        .ss_pin         = NRFX_SPIM_PIN_NOT_USED,
        .ss_active_high = false,
        .irq_priority   = NRFX_SPIM_DEFAULT_CONFIG_IRQ_PRIORITY,
        .orc            = 0xFF,
        .frequency      = NRF_SPIM_FREQ_250K,
        .mode           = (mode == 3) ? NRF_SPIM_MODE_3 : NRF_SPIM_MODE_0,
        .bit_order      = NRF_SPIM_BIT_ORDER_MSB_FIRST,
    };
    const nrfx_spim_xfer_desc_t xfer = NRFX_SPIM_XFER_TRX(tx, tx_len, rx, rx_len);

    /* Blocking mode: no handler, nrfx_spim_xfer returns after EVENTS_END.
     * It fails only for a buffer outside RAM, which spi_bus.h forbids. */
    if (nrfx_spim_init(&s_spim, &cfg, NULL, NULL) != NRFX_SUCCESS)
    {
        return;
    }
    nrf_gpio_pin_clear(cs_pin);
    (void)nrfx_spim_xfer(&s_spim, &xfer, 0);
    nrf_gpio_pin_set(cs_pin);
    nrfx_spim_uninit(&s_spim);
    /* uninit resets only MISO; SCK and MOSI stay outputs at their last level. */
    bus_idle_high();
}
