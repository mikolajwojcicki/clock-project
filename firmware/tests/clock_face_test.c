/*
 * Host check for face glyphs: prints every glyph and asserts they all differ.
 *   gcc -Wall -Wextra -I firmware/src firmware/tests/clock_face_test.c \
 *       firmware/src/clock_face.c -o /tmp/clock_face_test && /tmp/clock_face_test
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "clock_face.h"

int main(void)
{
    const char set[] = "0123456789:PNWTSRCZDO";
    size_t n = strlen(set);

    for (size_t i = 0; i < n; i++)
    {
        const uint8_t *g = clock_face_glyph(set[i]);
        printf("'%c'\n", set[i]);
        for (int r = 0; r < 7; r++)
        {
            for (int c = 4; c >= 0; c--)
            {
                putchar((g[r] >> c) & 1 ? '#' : '.');
            }
            putchar('\n');
        }
    }
    for (size_t i = 0; i < n; i++)
    {
        for (size_t j = i + 1; j < n; j++)
        {
            if (memcmp(clock_face_glyph(set[i]), clock_face_glyph(set[j]), 7) == 0)
            {
                printf("clash: '%c' == '%c'\n", set[i], set[j]);
                assert(0);
            }
        }
    }
    puts("clock_face_test ok");
    return 0;
}
