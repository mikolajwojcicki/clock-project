#ifndef CLOCK_FACE_H
#define CLOCK_FACE_H

#include <stdbool.h>
#include <stdint.h>

#include "clock.h"

/** 122 x 250, same axes as the Waveshare 2.13" V4 panel. True means black. */
bool clock_face_black(uint32_t x, uint32_t y, const clock_time *t);

#endif
