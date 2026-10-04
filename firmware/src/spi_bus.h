#ifndef SPI_BUS_H
#define SPI_BUS_H

#include <stddef.h>
#include <stdint.h>

void spi_bus_init(void);

/**
 * One SPIM0 transfer at 250 kHz with cs_pin held low; mode is 0 or 3.
 * tx and rx must be in RAM (EasyDMA cannot read flash), each at most 255 bytes.
 * Do not use rx_len == 1: nRF52832 anomaly 58 clocks out an extra byte.
 */
void spi_bus_transfer(uint32_t cs_pin, uint8_t mode,
                      const uint8_t *tx, size_t tx_len,
                      uint8_t *rx, size_t rx_len);

#endif
