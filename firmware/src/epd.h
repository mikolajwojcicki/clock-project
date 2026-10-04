#ifndef EPD_H
#define EPD_H

#include <stdbool.h>

/**
 * Waveshare 2.13" V4 (SSD1680, 122 x 250): reset, full refresh with a black
 * border and a 16 px checkerboard, then deep sleep.
 * Returns false if EPD_BUSY did not clear within 10 s.
 */
bool epd_show_test_pattern(void);

#endif
