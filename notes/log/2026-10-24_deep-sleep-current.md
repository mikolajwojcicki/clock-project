# 2026-10-24 — deep-sleep current measurement (PPK2)

- **Date:** 2026-10-24
- **Done:** task 4.0 leak check. Runs A, B and C follow (see
  `openspec/changes/add-deep-sleep-and-current-measurement/design.md`).
- **Failed:** —
- **Fix:** —
- **Why this part or library:** the PPK2 on P22 supplies only `VDD_nRF`. The
  display, LIS3DH and DCF module run from the USB-fed DK `VDD`, so a P22-only
  run neither isolates the MCU nor counts the clock.
- **Evidence:** this note; CSVs under `notes/measurements/` as the runs land.

## 4.0 Leak check

Setup: DK on USB, PPK2 disconnected from P22 (SB9 cut), breadboard wired as in
`hardware/docs/breadboard-wiring-table.md`, multimeter to DK GND.

| Point | Voltage |
| --- | --- |
| nRF side of P22 (`VDD_nRF`, no supply connected) | ~1.5 V |
| DK P1 `VDD` | ~3.0 V |

Result: `VDD_nRF` sits at about half of `VDD` with nothing supplying it, so the
nRF is back-powered. The DK LEDs and the LIS3DH board LED light in this state.
The current comes from the USB `VDD` domain through I/O pins and their ESD
diodes. Which pins carry it (breadboard lines, DK LEDs or the interface UART)
is not yet separated; run A answers part of that.

Decision: the PPK2 Source Meter runs at **3.0 V** in A and C, equal to DK
`VDD`, so no line from the `VDD` domain sits above `VDD_nRF` in C.

Repeat with every breadboard wire unplugged from the DK headers: the nRF side
of P22 still reads ~1.5 V. The breadboard is not the leak path; the source is
on the DK. Candidates are the four LEDs (anode on DK `VDD`, cathode on an nRF
GPIO, so LED + pin ESD diode is a forward path into `VDD_nRF`) and the
interface UART. The LEDs light in this state, so the LED path is confirmed
active; whether the UART adds to it is not separated. Roughly 3.0 V minus an
LED drop minus an ESD diode drop gives about 1.5 V, which fits the LED path.

Consequence for run A: the leak needs `VDD_nRF` below DK `VDD`. With the
PPK2 sourcing 3.0 V on `VDD_nRF` and DK `VDD` at ~3.0 V, the LED path has no
forward voltage and should stop. Check during run A: the DK LEDs go dark once
the PPK2 is on, unless firmware drives them.
