# Research brief: isolating the nRF52832 from the DK interface MCU (SB20-SB27)

Scratch workspace. Parent repo rules apply (`../../AGENTS.md`): NixOS, ask the user
only via the `ask` tool, UART only via nix-shell + pyserial, commit work in the
parent repo with thesis-readable messages. This directory holds research output
only. Do not edit firmware, hardware or thesis files from here.

## Why this exists

Run A7 (`notes/log/2026-10-24_deep-sleep-current.md`, commit `b109b4c`) showed the
idle current seen by the PPK2 on `P22` depends on supply voltage:
3.3 V -> 16.3 uA, 3.2 -> 8.5, 3.1 -> 2.5, 3.0 -> 0.66, 2.9 -> 0.30 (mean, µA).
The chip should draw about 2 uA. So current flows between `VDD_nRF` and the
USB-powered interface MCU. The user considers cutting solder bridges SB20-SB27
on the nRF52 DK (PCA10040 **v3.0.1**) to isolate it. A cut is hard to undo and
may remove the on-board flash path. This research decides whether and how.

## Goal

Produce `PLAN.md` in this directory: a go/no-go and a step-by-step procedure the
user can follow, each step tied to a cited source. It must answer:

1. **Which bridges, exactly.** For each of SB20-SB27 (and any other bridge on
   the nRF-to-interface-MCU path, e.g. SB9 already cut, SB10, SB11 etc.):
   signal, default state (closed/open), what cutting does, location on the
   v3.0.1 board. Cite DK guide section/page and the schematic if available.
2. **Is cutting all of them needed?** Minimal set that removes the leak. Which
   signals actually carry current from the interface MCU into nRF pins (SWD
   pull-ups, RESET, UART, SWO, others such as the interface MCU driving `P0.xx`
   LEDs/buttons)? Rank by likelihood. Note LED2 on `P0.18` stays dimly lit in
   every firmware: relate to SB27/SWO.
3. **Non-destructive alternatives first.** Can the leak be tested or removed
   without a knife? e.g. holding the interface MCU in reset, power-switch or
   "nRF only" mode of the DK, `P20`/`P22`/`P21` header usage, external supply
   on `VDD_nRF` with SB9 cut (already), battery powered DK with USB unplugged
   plus external SWD only for flashing. Say which the guide documents for
   current measurement and why our USB-connected setup was chosen
   (pin reset hangs with USB unplugged, see the note "USB unplugged").
4. **Flashing and reset after the cut.** With SWD (SB20/SB21) and RESET (SB26)
   cut, how do we program? Options to evaluate: external SWD probe on the debug
   header (which connector, which pins, 10-pin vs "Debug out" P20), J-Link
   OB re-routed to an external target, OpenOCD config in
   `firmware/apps/clock/armgcc/Makefile` (targets `flash`, `erase`, uses
   `interface/jlink.cfg`, serial via `OPENOCD_SERIAL`). Which probe do we have?
   (Check with `lsusb`; do not assume.) How is reset done without `P0.21`
   connected to the interface MCU?
5. **Revert.** For every cut: how to restore it (solder blob, 0 ohm link,
   wire), what tool and technique, how to verify continuity with a multimeter,
   and the expected readings before and after. If a cut cannot be restored
   reliably, say so plainly.
6. **Verification plan.** Before cutting: continuity readings to record per
   bridge. After cutting: a go/no-go sequence (flash works, `P0.22` marker on
   PPK2 `D0` behaves, idle capture at 3.0 V and 3.3 V). Success criterion: idle
   current no longer depends on supply voltage and sits near the datasheet
   figure (about 1.9-2.2 uA with RAM retained and RTC1 on LFXO; nRF52832 PS
   v1.9 section 17.1.1). Define what result means "leak not fixed, restore".
7. **Risks.** What can brick the DK or lose debug access; what cannot.

## Grounding rules (mandatory)

- Every claim in `PLAN.md` carries a source: PDF + section/page, URL, or a
  measured file. Mark anything unsourced `[INFERENCE]`. Never invent bridge
  numbers, pin names or board locations.
- Primary sources first:
  - `../../temp-resources/docs/nrf/nRF52_DK_User_Guide_v3.x.x.pdf`
    (solder bridge table, section on current measurement, debug out)
  - `../../temp-resources/docs/nrf/nRF52832_PS_v1.9.pdf` (17.1.1 sleep current,
    pin protection, `RESET` pin, SWD pins)
  - Nordic DK schematic / Altium/PDF for PCA10040 v3.0.1 (search the web, save
    the link and the file name; download into this directory if allowed).
  - Nordic DevZone threads on "nRF52 DK current measurement leakage", "SB cut",
    "interface MCU back-feed". Quote the thread URL and the answer, note date.
- PDFs: `nix-shell -p poppler-utils` then `pdftotext -layout`. Or the `pdf_*`
  MCP tools.
- Project facts come from files, not memory: `../../notes/log/2026-10-24_deep-sleep-current.md`,
  `../../hardware/docs/breadboard-wiring-table.md`, `../../hardware/docs/perfboard-shield.md`,
  `../../firmware/apps/clock/armgcc/Makefile`.
- Measured evidence: captures in `../../notes/measurements/*.ppkz` (unpack with
  `../../firmware/tools/ppk-plot/ppk-archive.sh`; numpy needs
  `nix-shell -p python3 stdenv.cc.cc.lib zlib` and `LD_LIBRARY_PATH` from
  `NIX_LDFLAGS`). Re-derive any number you quote; do not copy it blindly.
- Read-only on hardware. This session does not ask the user to cut or solder.
  It may ask for **non-destructive** bench checks (multimeter continuity, voltage
  readings, `lsusb`, an OpenOCD `init; exit` probe). Ask through `ask`, one
  concrete reading per request, and log answers in `evidence.md`.

## Method (autoresearch loop)

1. Write `QUESTIONS.md`: the 7 items above, each with status open/answered.
2. Loop per open question: search, read the primary source, extract the quote,
   write it to `evidence.md` with source + page, then update status. Prefer
   disconfirming evidence: for each candidate "cut X" ask what would show X is
   NOT the leak, and look for it.
3. Cross-check measured A7 numbers against the proposed leak path. Estimate
   the leak: 8 uA per 0.1 V is about 12 Mohm effective; say which pull-up/ESD
   path could plausibly look like that `[INFERENCE]` unless sourced.
4. Draft `PLAN.md`. Self-review: every step has a source, a revert, and a check.
5. Stop and hand over. Do not execute any hardware change.

## Deliverables in this directory

- `QUESTIONS.md`, `evidence.md` (quotes + sources + bench readings), `PLAN.md`
  (go/no-go, cut list, flash procedure, revert procedure, verification, risks).
- A short "pre-cut checklist" the user can print: readings to record,
  tools needed (iron, wick, knife, multimeter, probe), photos to take of each
  bridge before cutting.

## Environment facts

- Board: nRF52 DK PCA10040 v3.0.1; PPK2 S/N `FACEF4EE8A0A`; SB9 already cut,
  PPK2 Source Meter on `P22`.
- Current flash: `cd ../../firmware/apps/clock/armgcc && make SLEEP_UART=0 flash ACM=/dev/ttyACM0`
  (OpenOCD + J-Link OB on the DK). `make erase` keeps debug access.
- Firmware on chip: clock app `c7edb52`, no UART, pin reset enabled
  (`P0.21` as RESET). `P0.22` is a marker pin (PPK2 `D0`).
- No Zephyr, no RTOS, no imperative package managers. Use `nix-shell -p`.
