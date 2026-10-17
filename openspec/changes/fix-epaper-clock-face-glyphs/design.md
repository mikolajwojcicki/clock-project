## Context

`clock_face.c` has `k_digit[10][7]` (5 pixels wide, bit 4 is the left pixel), `k_colon`, and `letter()` with ten letters. `hit()` scales each bitmap pixel by 3 (day, seconds) or 4 (HH:MM). The day letters are uppercase only because `letter()` has no lowercase. The bitmaps are the only thing wrong; `text_hit`, `text_width` and layout are fine.

Observed digit bytes that cause trouble: `8 = 1E 11 11 1E 11 11 1E` is the letter `B`; `0/6/9/2/3/5` use `1E` / `1F` rows at the corners.

## Goals / Non-Goals

**Goals:** clear digits and letters at 5x7 with scale 3 and 4; a host render for review; a pairwise-difference check.

**Non-Goals:** a bigger or proportional font, a new layout, anti-aliasing, lowercase, a font library.

## Decisions

1. **Keep the 5x7 format and the scales.** The glyph tables are the only code that changes. Reason: no layout change, no RAM or flash cost.
2. **Round corners.** Proposed rows (hex, top to bottom): `0`: `0E 11 13 15 19 11 0E`; `2`: `0E 11 01 02 04 08 1F`; `3`: `0E 11 01 06 01 11 0E`; `4`: `02 06 0A 12 1F 02 02`; `5`: `1F 10 1E 01 01 11 0E`; `6`: `06 08 10 1E 11 11 0E`; `8`: `0E 11 11 0E 11 11 0E`; `9`: `0E 11 11 0F 01 02 0C`. `1` and `7` stay unless the render shows a problem. A slashed `0` is used so `0` and `O` differ; `O` stays round.
3. **Host render check.** `firmware/tests/clock_face_test.c` prints each glyph as `#` and `.` and asserts pairwise inequality. It needs `clock_face.c` to export a glyph accessor; `static` stays for everything else. Run line goes in `firmware/README.md` next to the other host checks.
4. **Review by the author on the board.** Real e-paper pixels differ from a terminal render. The photo is the acceptance proof.

## Risks / Trade-offs

- Some 5x7 shapes stay ambiguous at this size (`5` vs `S`, `0` vs `O`). Decision 2 handles `0`/`O`; the render and the photo show if `5`/`S` also needs a change.
- Exporting a glyph accessor adds a small API for a test only. Accepted: the alternative is testing through `clock_face_black` with pixel math, which is harder to read.
