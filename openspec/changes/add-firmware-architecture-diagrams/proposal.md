# Proposal

## Why

The only firmware diagram is `firmware/docs/software-flowchart.puml`, one coarse event-loop chart. The thesis firmware chapter and the promotor review need the clock firmware explained from overview down to register and ISR level, and BLE needs its own explanation (what the SoftDevice is, how `ble_sched.c` uses it, how the Web Bluetooth page talks to it). Today that knowledge lives only in code and scattered specs.

## What Changes

- Add three PlantUML diagrams of the same clock firmware (`firmware/apps/clock/src/main.c` plus `firmware/src/*`) at three zoom levels:
  - L1 overview: power states and wake events.
  - L2 module flow: main loop, ISR/handler sources, DCF-77, e-paper, alarm, motion, schedule store, BLE.
  - L3 low level: per-handler flow, peripherals (RTC, GPIOTE, TIMER, SPI, SAADC if used), sleep entry, SoftDevice interaction.
- Add one BLE swimlane diagram (web app, browser/OS BLE, SoftDevice, `ble_sched.c`, schedule store) and one BLE sequence diagram of a schedule read/write.
- Diagrams are derived **as-built from source**; any mismatch with `openspec/specs/` is recorded, not silently resolved.
- Add `firmware/docs/diagrams/README.md` indexing the diagrams and giving the render command (PlantUML podman container, SVG then PDF).
- Draft Polish thesis captions per diagram (draft only; author rewrites and logs in `notes/ai-use.md`).
- Existing `software-flowchart.*` stays untouched.

No firmware, web, or thesis-LaTeX edits.

## Capabilities

### New Capabilities
- `firmware-diagrams`: contract for the diagram set: levels, BLE views, as-built accuracy, reproducible PlantUML render, and captions.

### Modified Capabilities

## Impact

- New files under `firmware/docs/diagrams/` (`.puml`, `.svg`, `.pdf`, `README.md`, captions file).
- Read-only inspection of `firmware/apps/clock/src/main.c`, `firmware/src/*.c`, `web/app.js`, `sdk_config.h`.
- Thesis copies of the exports are a later, separate step (`clock-project-latex` submodule).
- `notes/ai-use.md` gets an entry when captions are drafted.
