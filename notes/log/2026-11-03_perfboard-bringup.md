# 2026-11-03 perfboard shield bring-up

Change `build-perfboard-prototype`. Shield schematic: `hardware/kicad/dk-perfboard-shield.kicad_sch` (v1.0-shield).

## 3.1 Header test-fit (unsoldered)

Parts owned: long-pin female (stacking) headers, standard male headers, standard female headers. The perfboard cannot take all four DK headers as they are, for two reasons found from the Nordic pick-and-place file:

- P4 (1x10) pins are 0.4 hole (1.016 mm) off the grid that P1, P2 and P3 share.
- The DK `P20` pin list (1x8) sits four holes below P4 and touched the perfboard underside.

Modifications the author made, as reported:

| Item | Change | Why |
| --- | --- | --- |
| Shield P4 (long-pin female) | Tails bent to the DK grid | Stagger fix |
| Shield P2 | Cut down from an 8-pin long-pin header | P2 is 1x6 |
| Second cut-down header (reported as "P6", 10-pin source) | Cut down from a 10-pin long-pin header | Header size to confirm; DK `P3` is 1x8, DK `P6` is the 1x9 AREF socket |
| DK `P20` pins | Shortened | Stop touching the perfboard; still reachable with a modified Dupont jumper |
| DK `P5` pins (2x3) | Shortened | Same reason |

Result: the author reports that the headers now fit. **Open:** photo not yet in `notes/media/`; the unconfirmed header above.

DK pins were cut, so `P20` and `P5` are only reachable with the modified Dupont jumper from now on. Layout keeps both zones free of parts and solder joints (design risk list, task 4.3).
