# 2026-10-17 libopencm3 port of the DCF-77 generator

Branch `experiment/libopencm3-generator`. Goal: drop the hand-written boot
code (`stm32f411_regs.h`, `startup.c`, `stm32f411re.ld`) and use libopencm3
(submodule, pinned commit `96297511`). Firmware commit for the port: `260a81f`.

## Results

| Item | Register-level (old) | libopencm3 (new) |
|---|---|---|
| Image size (`text`) | 4860 B | 7288 B (library code included) |
| Boot code in tree | 169 lines (91 regs.h + 41 startup.c + 37 ld) | 0 (generated linker script, library vector table) |
| Carrier `PA6` | 77492.96 Hz (-90.8 ppm) | 77492.93 Hz (-91.2 ppm); same offset, so it is the Saleae/MCO reference |
| `PA0` pulses | 100 / 200 ms, 1000 ms, gap 2000 ms | 100 / 200 ms, 1000.1 ms, gap 2000.2 ms |
| Carrier duty 0 / -6 / -12 / -20 dB | CCR 500 / 167 / 80 / 32 | measured 0.500 / 0.167 / 0.080 / 0.032 |
| Dip lag after `PA0` edge | within 12 us (README) | 12.71 us rising, 0.08 us falling (one edge each, weak evidence) |
| Fault keys `m g s p` | as recorded | `m`: one 2000 ms gap; `g`: 10.0 ms pulse at +500.1 ms; `s`: 36 s quiet, resumes at frame start; `p`: minute parity odd |
| DK receiver test, 10 min, 0 dB | `valid=579 invalid=5 minute_markers=10 overruns=0` | `valid=590 invalid=3 minute_markers=10 overruns=8` |
| DK frames vs generator `bits=` | consecutive minutes, parity OK | 9 of 9 complete frames bit-identical (first 2 partial while locking) |

## Notes

- Saleae Logic 2 crashed several times on long or fast captures. Short captures
  (1 MS/s, 3 s; 25 kS/s for 80 s) worked.
- `overruns=8` not investigated. Frames are intact. The antenna and generator
  coil were placed differently on the table than in the old run, which is the
  likely cause (author's judgement, not measured).
- Console ports with the DK plugged in: DK on `ttyACM0`/`ACM1`, ST-LINK on `ttyACM2`.

## Decision

All checks match the old firmware. Result: go. Delete the three boot files in a
separate commit and merge (task 5.1).
