# 2026-10-17 — e-paper clock-face glyphs redrawn

OpenSpec change `fix-epaper-clock-face-glyphs`, tasks 1.1-4.1.

Problem: on the face some digits read as letters. `8` was `1E 11 11 1E 11 11 1E`, which is the letter `B`. `0/2/3/5/6/9` had square corners and `4` had no diagonal.

Fix: 5x7 bitmaps in `firmware/src/clock_face.c` (`k_digit[]`, `letter()`); layout, scale 3/4 and refresh logic unchanged. Rows are hex, top to bottom; `#` is a black pixel.

| Glyph | Before | After |
|---|---|---|
| `0` | `0E 11 11 11 11 11 0E` (same as `O`) | `0E 11 13 15 19 11 0E` (slashed) |
| `2` | `1E 01 01 1E 10 10 1F` | `0E 11 01 02 04 08 1F` |
| `3` | `1E 01 01 0E 01 01 1E` | `0E 11 01 06 01 11 0E` |
| `4` | `11 11 11 1F 01 01 01` | `02 06 0A 12 1F 02 02` |
| `5` | `1F 10 10 1E 01 01 1E` | `1F 10 1E 01 01 11 0E` |
| `6` | `1E 10 10 1E 11 11 1E` | `06 08 10 1E 11 11 0E` |
| `8` | `1E 11 11 1E 11 11 1E` (= `B`) | `0E 11 11 0E 11 11 0E` |
| `9` | `1E 11 11 1E 01 01 1E` | `0E 11 11 0F 01 02 0C` |
| `S` | `0F 10 10 0E 01 01 1E` (close to `5`) | `0E 11 10 0E 01 11 0E` |

`1`, `7`, the colon and the other weekday letters (`P N W T R C Z D O`) were kept; the render showed no clash.

Host check: `firmware/tests/clock_face_test.c` prints every glyph and asserts all of `0-9 : P N W T S R C Z D O` are pairwise different. It needed the exported accessor `clock_face_glyph()`; the alternative, pixel math through `clock_face_black()`, is harder to read. Passes under `nix-shell -p gcc` (run line in `firmware/README.md`).

On the board: `make -C firmware/apps/clock` builds and flashes (firmware commit `9fa8b3a`; the later commits only touch the README and the task list). The author checked `88:88` and weekdays on the e-paper: glyphs correct so far. Photo: `notes/media/2026-10-17_clock_epaper-glyphs.jpg`.
