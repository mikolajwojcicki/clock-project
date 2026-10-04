# Proposal

## Why

The prototype guide says "DK `3V3`" and "DK `GND`", but the nRF52 DK has no pin
with that label. It has several `VDD`, `GND`, `5V`, `VDD_nRF`, and
`External supply` pins, and a beginner cannot tell which one is a safe 3.3 V
output. The nRF52 DK User Guide v3.x.x also shows that the current pin map
puts SPI on the DK buttons (`P0.13`–`P0.15`), display control on the DK LEDs
and I/O-expander interrupt (`P0.17`–`P0.20`), and the LIS3DH chip select on
`P0.21`, which the blinky build configures as the reset pin
(`CONFIG_GPIO_AS_PINRESET`). Those conflicts can corrupt SPI traffic, load the
display `BUSY` output, and make the sensor chip select unusable.

## What Changes

- Name the exact DK power source for the breadboard: connector `P1`, pin
  labeled `VDD`, and a `GND` pin on `P1`. State that all `VDD` pins on `P1`
  are one net and all `GND` pins are one net.
- Add a "pins you must not use for power" table: `P1 5V`, `External supply`
  (`P21`, an input), `nRF current measurement` (`P22`), `VDD_nRF`, the coin
  cell holder, and the Debug out connector.
- Explain that USB power reaches `VDD` through a reverse-protection diode, so
  the rail is slightly below 3.3 V, and require the user to measure and record
  it.
- **BREAKING (wiring):** replace the pin map with DK pins that have no
  on-board load: SPI on `P0.02`–`P0.04`, display `CS` and `DC` on `P0.11` and
  `P0.12`, and all slow signals on `P0.22`–`P0.25` and `P0.28`–`P0.31`.
  Anyone who already wired the old map must rewire.
- List DK pins that are reserved by board hardware and must stay unused:
  `P0.00`/`P0.01`, `P0.05`–`P0.08`, `P0.09`/`P0.10`, `P0.13`–`P0.20`,
  `P0.21`, `P0.26`/`P0.27`.
- Add a DK revision check (`PCA10040`, version sticker 3.x.x) because the
  connector and solder-bridge data come from the v3.x.x user guide.
- Update the KiCad breadboard schematic DK symbol pin names and the exported
  PDF to match the new guide.
- Cite the nRF52 DK User Guide v3.x.x and nRF52832 Product Specification v1.9
  in the guide references.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `breadboard-hardware-prototype`: the wiring requirement changes from "SPI in
  `P0.11`–`P0.20`" to "DK pins with no on-board button, LED, UART, NFC,
  crystal, reset, or I/O-expander connection", and it gains an explicit DK
  power-pin identification requirement.

## Impact

- `hardware/docs/breadboard-prototype-guide.md` (power rules, rail table,
  pin map, wiring steps, checklist, test record, references)
- `hardware/kicad/breadboard-prototype-symbols.kicad_sym`,
  `hardware/kicad/breadboard-prototype.kicad_sch`,
  `hardware/kicad/breadboard-prototype.pdf`
- `openspec/specs/breadboard-hardware-prototype/spec.md` (via delta)
- No firmware changes. Blinky keeps using the DK LEDs, which the new map
  leaves free. Future driver firmware must use the new pin map.
- The custom MDBT42Q PCB pin map in `AGENTS.md` is out of scope; the DK map is
  DK-specific.
