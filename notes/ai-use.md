# AI use register

The thesis section "Informacja o wykorzystaniu narzędzi AI" is written only by the author. This file is the raw log. Agents do not turn it into that section.

| Date | Tool | Purpose | Check or rewrite |
| --- | --- | --- | --- |
| 2026-07-19 | Cursor agent | Scaffold monorepo, WIT LaTeX submodule, blinky app, PCB BOM | Author reviews commits; blinky later flashed on the DK |
| 2026-07-26 | Cursor agent | Spec Kit, then project constitution | Replaced by OpenSpec on 2026-09-12 |
| 2026-09-12 | Cursor agent | Breadboard prototype guide and OpenSpec migration | Guide used on the bench in October |
| 2026-09-13 | Cursor agent | Inventory reconciliation, safety edits, KiCad breadboard schematic | Schematic matched to the wired prototype |
| 2026-10-04 | Cursor agent | DK pin remap, bring-up firmware (e-paper, LIS3DH, DCF capture, buzzer) | DK logs: safe states, buttons, LIS3DH ID and INT1, e-paper pattern, buzzer |
| 2026-10-06 | Cursor agent | Nucleo DCF-77 frame generator and 77.5 kHz carrier | Analyzer timing and DK decode logged in firmware README |
| 2026-10-13 | Cursor agent | Thesis roadmap and this evidence log from the promotor Teams dump | Author set the deadline assumption and the rewrite rule |
| 2026-10-13 | Cursor agent | Clock UART (`app_uart`/UARTE), SPI EVENTS_END hang, SSD1680 dual-RAM upright refresh, digit/weekday glyphs | Author verified on DK: time-set echo, minute updates stay upright with ribbon at bottom |
| 2026-10-24 | Cursor agent | PPK2 capture archive tool: `ppk_archive.py` and `ppkz.py` in `firmware/tools/ppk-plot/` (pack CSV to zstd `.ppkz` + sidecar, unpack, `ppk-plot` input) | Unit tests in `test_ppk_archive.py` (both CSV layouts, gap, repeated timestamp, bad header, float64 fallback, window, forced mismatch leaves no output); pack reads every sample back against the CSV; unpack of the real 96 s capture is byte-identical to its CSV; `ppk-plot --mean` equal for CSV and `.ppkz` |
| 2026-10-27 | Claude Code agent | Slide text for `presentation.tex` (seminar deck) | Author must rewrite every frame in own words before presenting; numbers checked against the 2026-10-24 log |
| 2026-10-28 | Claude Code agent | Phase analysis of runs A9, B1, B2, D, C7, C8 and the log note `notes/log/2026-10-28_deep-sleep-current.md` (tables, C vs A + B, daily energy) | Author ran the six captures; numbers come from throwaway scripts over the `.ppkz` packs; lab log, not thesis prose; rewrite before any reuse in the thesis |
| 2026-10-28 | Cursor agent | Firmware fix `d0685ee` (refresh spin: per-pixel `snprintf`, `arm_wake` while drawing); SWD PC proof; log addendum A10/A9b, revised daily energy; DK logic back-feed noted as open item 5 | Author capture `ppk-20261028T212701`; solder-bridge research left to separate autoresearch |

No thesis chapter prose has been drafted yet. When a draft is kept, add a row and rewrite it before it stays in the PDF.
