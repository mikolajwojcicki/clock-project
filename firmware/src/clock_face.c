#include "clock_face.h"

#define FACE_W 122
#define FACE_H 250

/* 5x7, bit 4 is the left pixel. */
static const uint8_t k_digit[10][7] = {
    {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}, /* slashed: differs from O */
    {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E},
    {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F},
    {0x0E, 0x11, 0x01, 0x06, 0x01, 0x11, 0x0E},
    {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02},
    {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E},
    {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E},
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08},
    {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E},
    {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C},
};

static const uint8_t k_colon[7] = {0x00, 0x04, 0x04, 0x00, 0x04, 0x04, 0x00};

static const uint8_t *letter(char c)
{
    static const uint8_t P[7] = {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10};
    static const uint8_t N[7] = {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11};
    static const uint8_t W[7] = {0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11};
    static const uint8_t T[7] = {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04};
    static const uint8_t S[7] = {0x0E, 0x11, 0x10, 0x0E, 0x01, 0x11, 0x0E};
    static const uint8_t R[7] = {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11};
    static const uint8_t C[7] = {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E};
    static const uint8_t Z[7] = {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F};
    static const uint8_t D[7] = {0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C};
    static const uint8_t O[7] = {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
    switch (c)
    {
    case 'P': return P;
    case 'N': return N;
    case 'W': return W;
    case 'T': return T;
    case 'S': return S;
    case 'R': return R;
    case 'C': return C;
    case 'Z': return Z;
    case 'D': return D;
    case 'O': return O;
    default: return k_colon;
    }
}

const uint8_t *clock_face_glyph(char c)
{
    if (c >= '0' && c <= '9')
    {
        return k_digit[c - '0'];
    }
    if (c == ':')
    {
        return k_colon;
    }
    if (c == ' ')
    {
        return k_colon;
    }
    return letter(c);
}

static bool hit(char c, uint32_t ox, uint32_t oy, uint32_t scale, uint32_t x, uint32_t y)
{
    if (c == ' ' || x < ox || y < oy)
    {
        return false;
    }
    uint32_t lx = (x - ox) / scale;
    uint32_t ly = (y - oy) / scale;
    if (lx >= 5 || ly >= 7)
    {
        return false;
    }
    return (clock_face_glyph(c)[ly] >> (4 - lx)) & 1u;
}

static bool text_hit(const char *s, uint32_t ox, uint32_t oy, uint32_t scale, uint32_t x, uint32_t y)
{
    uint32_t advance = 5 * scale + scale;
    for (uint32_t i = 0; s[i] != '\0'; i++)
    {
        if (hit(s[i], ox + i * advance, oy, scale, x, y))
        {
            return true;
        }
    }
    return false;
}

static uint32_t text_width(const char *s, uint32_t scale)
{
    uint32_t n = 0;
    while (s[n] != '\0')
    {
        n++;
    }
    if (n == 0)
    {
        return 0;
    }
    return n * (5 * scale + scale) - scale;
}

bool clock_face_black(uint32_t x, uint32_t y, const clock_time *t)
{
    if (x >= FACE_W || y >= FACE_H)
    {
        return false;
    }
    if (x < 2 || y < 2 || x >= FACE_W - 2 || y >= FACE_H - 2)
    {
        return true;
    }

    /* Uppercase only: letter() has no lowercase, so "Wt" drew as "W:". */
    static const char *abbr[] = {"PN", "WT", "SR", "CZ", "PT", "SO", "ND"};
    const char *day = (t->wday >= 0 && t->wday <= 6) ? abbr[t->wday] : "??";
    /* Called for every pixel of every RAM write: snprintf here kept the CPU
     * busy for about 8 s per refresh (2026-10-28 PC samples). Fields are 0-59. */
    char hm[6] = {
        (char)('0' + t->hour / 10), (char)('0' + t->hour % 10), ':',
        (char)('0' + t->min / 10), (char)('0' + t->min % 10), '\0',
    };
    char sec[3] = {(char)('0' + t->sec / 10), (char)('0' + t->sec % 10), '\0'};

    uint32_t day_w = text_width(day, 3);
    uint32_t hm_w = text_width(hm, 4);
    uint32_t sec_w = text_width(sec, 3);
    if (text_hit(day, (FACE_W - day_w) / 2, 40, 3, x, y))
    {
        return true;
    }
    if (text_hit(hm, (FACE_W - hm_w) / 2, 100, 4, x, y))
    {
        return true;
    }
    if (text_hit(sec, (FACE_W - sec_w) / 2, 170, 3, x, y))
    {
        return true;
    }
    return false;
}
