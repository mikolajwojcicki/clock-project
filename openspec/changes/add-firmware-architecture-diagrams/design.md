# Design

## Context

Firmware: bare-metal nRF52832, nRF5 SDK 17.1.0, SoftDevice for BLE. `firmware/apps/clock/src/main.c` (559 lines) is the app; modules live in `firmware/src/` (`alarm`, `ble_sched`, `board`, `buzzer`, `clock`, `clock_face`, `dcf77_classify`, `dcf77_sync`, `epd`, `lis3dh`, `motion_count`, `sched_codec`, `sched_store`, `spi_bus`). Web side: `web/app.js`, `web/schedule.js`. Existing diagram `firmware/docs/software-flowchart.puml` is rendered with PlantUML to SVG, then `rsvg-convert` to PDF (keeps Polish letters). Existing specs `ble-schedule-service`, `low-power-sleep-policy`, `dcf77-time-sync`, `display-refresh-policy`, `motion-dismiss`, `alarm-schedule` describe intended behavior.

## Goals / Non-Goals

**Goals:**
- Three consistent zoom levels plus one BLE diagram and one BLE sequence diagram.
- Facts traceable to source; each L3 node cites a function or file in a comment.

**Non-Goals:**
- No firmware, web, or LaTeX edits; no change to `software-flowchart.*`.
- No hardware schematic diagrams.

## Decisions

- **Format: PlantUML via the local `plantuml/plantuml-server` podman image, driven with `curl`.** User choice. The container runs as a server; sources are sent as HTTP requests and the SVG comes back in the response. Matches the existing `.puml` + SVG + PDF pipeline. Alternatives (Mermaid, TikZ, Graphviz) rejected by user. Task 1.1 starts the container (published port, removed afterwards) and fixes the exact request form (POST of the `.puml` text to `/svg`, or GET with the encoded source) against the existing flowchart. Fall back to the nix-shell `plantuml` only if the server cannot render, and say so in the index.
- **Location: `firmware/docs/diagrams/`.** Source of truth beside code; thesis copies later.
- **Files:** `l1-overview`, `l2-modules`, `l3-detail` (activity/state style), `ble-overview` (component/swimlane with SoftDevice), `ble-sequence` (sequence). Each `.puml` + `.svg` + `.pdf`. Monochrome, DejaVu Serif, dpi 160, same skin as the existing chart so figures look uniform in print.
- **L3 may split.** If one L3 chart is unreadable on A4, split into labeled parts (`l3-detail-a/b/...`) while keeping a single index entry. Decide after reading the code.
- **Accuracy method:** read the source first, build a fact table (handlers, ISRs, peripherals, timers, flash use, SoftDevice events handled), then draw. Mismatches against specs go into the index. Known prior finding: the code has no `s_events` bits; handlers run each pass and `arm_wake` sets the RTC1 deadline.
- **SoftDevice explanation** in the BLE diagram: precompiled Nordic binary at the bottom of flash, owns radio, timing-critical link layer, GAP/GATT/ATT, and reserved RAM/peripherals; app calls `sd_*` over SVC and receives events via `SD_EVT`/`BLE_EVT` handling. Facts about SoftDevice version, advertising parameters, MTU and queued-write buffer come from `sdk_config.h` and `ble_sched.c`, not from memory.
- **Captions:** Polish, one per diagram, in `firmware/docs/diagrams/captions.md`, labeled agent draft; `notes/ai-use.md` entry added.

## Risks / Trade-offs

- Exact curl request form depends on the server version (POST body vs encoded GET, size limits on large L3 sources) → settle in 1.1 with the existing flowchart and a large test; documented in the index.
- L3 detail makes the chart huge → allow split, keep node text short.
- Code changes after drawing make diagrams stale → README states the commit the diagrams were derived from.
- Spec/code drift found during reading → listed, not fixed here (separate change).

## Layout fix (2026-11-03)

Author feedback: L2 module diagrams look ugly, lines cross over themselves. Cause: `main` fans out to seven handlers and handlers fan into eleven modules; dashed ISR edges overlap solid edges; both parts are `left to right`, so Graphviz routes edges around each other.

- **Fewer edges:** remove the `main -> each handler` fan-out; show loop order once as an ordered node chain or note (`poll_uart`, `poll_clock`, `poll_alarm`, `poll_dcf`, `poll_ble`, `arm_wake`, `draw`). Each module node appears once per part.
- **Layered columns:** ISR | handler | module | hardware, `top to bottom` with hidden links to pin ranks; `skinparam linetype ortho` for straight segments.
- **Split by concern if crossings remain:** loop and ISR flags; alarm, motion, buzzer; DCF and clock; BLE and store; display. Each part labelled, one index entry per part.
- **Check:** render, then view the SVG/PNG and count crossings; no sign-off from source reading alone.
- **Risk:** `ortho` with edge labels is unreliable in Graphviz; fall back to `polyline` and move labels into node text.
