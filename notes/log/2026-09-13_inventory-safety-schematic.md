# 2026-09-13 — inventory, safety, schematic

Archived changes: `reconcile-hardware-inventory-and-bom`, `harden-prototype-guide-safety-and-verification`.

- **Done:** Owned tools and parts recorded. Guide gained stop conditions for power, e-paper SPI mode, meter use, DCF power-down, and buzzer flyback. KiCad breadboard schematic drawn and tidied.
- **Failed:** Guide still asked a beginner to guess DK power pins and some electrical details. High-quiescent regulator candidate and an unverified 5 V buzzer were rejected in the inventory pass.
- **Fix:** Explicit checks and a schematic instead of inferred wiring.
- **Why:** Reversed power or a floating DCF enable can damage the prototype before firmware exists.
- **Evidence:** `hardware/kicad/breadboard-prototype.kicad_sch`, `hardware/bom/`.
