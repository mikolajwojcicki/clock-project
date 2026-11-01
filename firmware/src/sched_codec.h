#ifndef SCHED_CODEC_H
#define SCHED_CODEC_H

#include <stddef.h>
#include <stdint.h>

#include "alarm.h"

/**
 * Wire format of the alarm table (BLE characteristic and flash record payload):
 * byte 0 = version (1), then ALARM_DAYS x ALARM_SLOTS u16 little-endian, Monday first.
 * Bit 15 = enabled, bits 0-10 = minute of day (0-1439).
 */
#define SCHED_VERSION 1u
#define SCHED_LEN (1u + ALARM_DAYS * ALARM_SLOTS * 2u)

typedef enum {
    SCHED_OK = 0,
    SCHED_ERR_LENGTH,
    SCHED_ERR_VERSION,
    SCHED_ERR_MINUTE,
    SCHED_ERR_DUPLICATE
} sched_status;

void sched_encode(const alarm_slot t[ALARM_DAYS][ALARM_SLOTS], uint8_t out[SCHED_LEN]);

/** Validates buf and fills out. On any error out is left untouched. */
sched_status sched_decode(const uint8_t *buf, size_t len, alarm_slot out[ALARM_DAYS][ALARM_SLOTS]);

#endif
