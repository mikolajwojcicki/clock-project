# Tasks

## 1. Render pipeline and fact table

- [ ] 1.1 Prove PlantUML rendering by `curl` against the local podman `plantuml/plantuml-server` container (start it with a published port, stop it afterwards): send `firmware/docs/software-flowchart.puml` and verify a valid SVG with Polish letters comes back; convert to PDF with `rsvg-convert` from nix-shell; record the working curl command for the README.
- [ ] 1.2 Read `firmware/apps/clock/src/main.c`, `firmware/src/*.c` and `sdk_config.h`; write a fact table (handlers, ISRs, peripherals, timers, SoftDevice events, flash writes, sleep entry) in `firmware/docs/diagrams/README.md` and verify each row names a file and function.

## 2. Firmware diagrams (three levels)

- [ ] 2.1 Draw `l1-overview.puml` (power states and wake events); render SVG and PDF and verify it shows every wake source from the fact table.
- [ ] 2.2 Draw `l2-modules.puml` (main loop, handlers, modules, interactions); verify every module in `firmware/src` used by the clock app appears.
- [ ] 2.3 Draw `l3-detail.puml` (per-handler decisions, peripherals, sleep entry/exit); split into labeled parts if unreadable on A4; verify each node's comment cites a source function.
- [ ] 2.4 Cross-check L1 against L2 against L3 for consistent names and states; verify by walking one alarm cycle and one DCF sync through all three.

## 3. Bluetooth diagrams

- [ ] 3.1 Read `firmware/src/ble_sched.c`, `sched_codec.c`, `sched_store.c`, `web/app.js`, `web/schedule.js`; add BLE facts (advertising window, UUIDs, MTU, long write, storage) to the fact table.
- [ ] 3.2 Draw `ble-overview.puml` with lanes web app, browser/OS BLE, SoftDevice, `ble_sched`, schedule store, and an explicit SoftDevice-role note; verify it covers advertising, connect, discovery, read, write, disconnect.
- [ ] 3.3 Draw `ble-sequence.puml` for a schedule read and a schedule write; verify message order matches `ble_sched.c` event handling.

## 4. Index, captions, drift list

- [ ] 4.1 Finish `README.md`: diagram index, regenerate commands (podman container then `rsvg-convert`), source commit, and a spec-vs-code mismatch list; verify the commands run as written from a clean checkout.
- [ ] 4.2 Write `captions.md` with one Polish agent-draft caption per diagram, and add an entry to `notes/ai-use.md`; verify one caption per exported diagram.
- [ ] 4.3 Final check: all `.puml` have `.svg` and `.pdf`, files open and are legible; commit in separate commits (tooling/render check, firmware diagrams, BLE diagrams, docs) with thesis-readable bodies.
