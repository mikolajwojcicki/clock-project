#ifndef LIS3DH_H
#define LIS3DH_H

#include <stdint.h>

#define LIS3DH_WHO_AM_I_VALUE 0x33

/* INT1_SRC bits */
#define LIS3DH_INT_IA 0x40
#define LIS3DH_INT_ZH 0x20
#define LIS3DH_INT_YH 0x08
#define LIS3DH_INT_XH 0x02

uint8_t lis3dh_who_am_i(void);

/** 100 Hz, +-2 g, high-passed X/Y/Z high events on INT1, about 256 mg, latched. */
void lis3dh_motion_int1_enable(void);

/** Read INT1_SRC; this also releases the latched INT1 line. */
uint8_t lis3dh_int1_source(void);

void lis3dh_power_down(void);

#endif
