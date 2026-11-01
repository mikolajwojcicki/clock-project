#include "sched_codec.h"

#include <string.h>

#define EN_BIT 0x8000u
#define MIN_MASK 0x07FFu

void sched_encode(const alarm_slot t[ALARM_DAYS][ALARM_SLOTS], uint8_t out[SCHED_LEN])
{
    out[0] = (uint8_t)SCHED_VERSION;
    uint8_t *p = out + 1;
    for (int d = 0; d < ALARM_DAYS; d++)
    {
        for (int s = 0; s < ALARM_SLOTS; s++)
        {
            uint16_t v = (uint16_t)(t[d][s].hour * 60u + t[d][s].min);
            if (t[d][s].enabled)
            {
                v |= EN_BIT;
            }
            *p++ = (uint8_t)(v & 0xFFu);
            *p++ = (uint8_t)(v >> 8);
        }
    }
}

sched_status sched_decode(const uint8_t *buf, size_t len, alarm_slot out[ALARM_DAYS][ALARM_SLOTS])
{
    if (len != SCHED_LEN)
    {
        return SCHED_ERR_LENGTH;
    }
    if (buf[0] != SCHED_VERSION)
    {
        return SCHED_ERR_VERSION;
    }
    alarm_slot tmp[ALARM_DAYS][ALARM_SLOTS];
    const uint8_t *p = buf + 1;
    for (int d = 0; d < ALARM_DAYS; d++)
    {
        for (int s = 0; s < ALARM_SLOTS; s++)
        {
            uint16_t v = (uint16_t)(p[0] | (p[1] << 8));
            p += 2;
            /* Bits 11-14 are reserved and must be zero, so a 12-bit minute cannot slip through. */
            uint16_t minute = v & (uint16_t)~EN_BIT;
            if (minute > 1439u)
            {
                return SCHED_ERR_MINUTE;
            }
            tmp[d][s].hour = (uint8_t)(minute / 60u);
            tmp[d][s].min = (uint8_t)(minute % 60u);
            tmp[d][s].enabled = (v & EN_BIT) != 0;
        }
        if (!alarm_day_valid(tmp[d]))
        {
            return SCHED_ERR_DUPLICATE;
        }
    }
    memcpy(out, tmp, sizeof tmp);
    return SCHED_OK;
}

static uint16_t crc16(const uint8_t *d, size_t n)
{
    uint16_t c = 0xFFFF;
    while (n--)
    {
        c ^= (uint16_t)(*d++) << 8;
        for (int i = 0; i < 8; i++)
        {
            c = (c & 0x8000) ? (uint16_t)((c << 1) ^ 0x1021) : (uint16_t)(c << 1);
        }
    }
    return c;
}

void sched_record_pack(const alarm_slot t[ALARM_DAYS][ALARM_SLOTS], uint8_t out[SCHED_RECORD_LEN])
{
    memset(out, 0, SCHED_RECORD_LEN);
    for (int i = 0; i < 4; i++)
    {
        out[i] = (uint8_t)(SCHED_MAGIC >> (8 * i));
    }
    sched_encode(t, out + 4);
    uint16_t c = crc16(out + 4, SCHED_LEN);
    out[4 + SCHED_LEN] = (uint8_t)c;
    out[5 + SCHED_LEN] = (uint8_t)(c >> 8);
}

sched_status sched_record_unpack(const uint8_t *rec, alarm_slot out[ALARM_DAYS][ALARM_SLOTS])
{
    uint32_t m = (uint32_t)rec[0] | (uint32_t)rec[1] << 8 | (uint32_t)rec[2] << 16 | (uint32_t)rec[3] << 24;
    uint16_t c = (uint16_t)(rec[4 + SCHED_LEN] | rec[5 + SCHED_LEN] << 8);
    if (m != SCHED_MAGIC || c != crc16(rec + 4, SCHED_LEN))
    {
        return SCHED_ERR_VERSION;
    }
    return sched_decode(rec + 4, SCHED_LEN, out);
}
