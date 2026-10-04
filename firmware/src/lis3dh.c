#include "lis3dh.h"

#include "dk_breadboard_pins.h"
#include "spi_bus.h"

#define REG_WHO_AM_I   0x0F
#define REG_CTRL1      0x20
#define REG_CTRL2      0x21
#define REG_CTRL3      0x22
#define REG_CTRL4      0x23
#define REG_CTRL5      0x24
#define REG_REFERENCE  0x26
#define REG_INT1_CFG   0x30
#define REG_INT1_SRC   0x31
#define REG_INT1_THS   0x32
#define REG_INT1_DUR   0x33

#define SPI_READ       0x80
#define LIS3DH_SPI_MODE 3

static uint8_t rd(uint8_t reg)
{
    uint8_t tx[2] = {reg | SPI_READ, 0xFF};
    uint8_t rx[2];
    spi_bus_transfer(SENSOR_CS, LIS3DH_SPI_MODE, tx, 2, rx, 2);
    return rx[1];
}

static void wr(uint8_t reg, uint8_t value)
{
    uint8_t tx[2] = {reg, value};
    spi_bus_transfer(SENSOR_CS, LIS3DH_SPI_MODE, tx, 2, NULL, 0);
}

uint8_t lis3dh_who_am_i(void)
{
    return rd(REG_WHO_AM_I);
}

void lis3dh_motion_int1_enable(void)
{
    wr(REG_CTRL1, 0x57);    /* 100 Hz, normal mode, X/Y/Z on */
    wr(REG_CTRL2, 0x01);    /* high-pass filter on interrupt 1, so gravity does not trigger */
    wr(REG_CTRL3, 0x40);    /* IA1 routed to INT1 */
    wr(REG_CTRL4, 0x00);    /* +-2 g */
    wr(REG_CTRL5, 0x08);    /* latch INT1 until INT1_SRC is read */
    wr(REG_INT1_THS, 0x10); /* 16 LSB x 16 mg = 256 mg */
    wr(REG_INT1_DUR, 0x00);
    (void)rd(REG_REFERENCE); /* reset the high-pass filter */
    wr(REG_INT1_CFG, 0x2A); /* OR of XH, YH, ZH */
    (void)rd(REG_INT1_SRC);
}

uint8_t lis3dh_int1_source(void)
{
    return rd(REG_INT1_SRC);
}

void lis3dh_power_down(void)
{
    wr(REG_INT1_CFG, 0x00);
    wr(REG_CTRL3, 0x00);
    wr(REG_CTRL1, 0x00);
    (void)rd(REG_INT1_SRC);
}
