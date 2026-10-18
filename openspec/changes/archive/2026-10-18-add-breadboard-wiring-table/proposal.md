## Why

The breadboard wiring has grown into a tangle of jumpers. The pin map exists (`hardware/docs/breadboard-prototype-guide.md` section 6), but it lists only DK pins, not both ends of every wire. Power, the buzzer stage and the shared SPI bus are not separated, so rewiring is slow and error-prone. A per-wire table that separates buses from single signals lets the board be rebuilt in a clean order. It also gives the perfboard layout (stage `build-perfboard-prototype`) a routing input.

## What Changes

- Add `hardware/docs/breadboard-wiring-table.md`: one row per wire with both endpoints, grouped as power, shared SPI bus, single signals and buzzer stage.
- Mark which signals are bus members (shared by more than one device) and which are point to point.
- State a rebuild order and a continuity check list.
- Pin names stay as in `firmware/boards/dk_breadboard_pins.h`; no firmware change.

## Capabilities

### New Capabilities
- `breadboard-wiring-table`: per-wire connection table with bus and signal classes.

### Modified Capabilities
<!-- none -->

## Impact

- `hardware/docs/` only. The guide section 6 stays the pin-map source and links to the new table.
- Locked architecture unchanged (bare-metal nRF52832, nRF5 SDK 17.1.0, no Zephyr or RTOS).
