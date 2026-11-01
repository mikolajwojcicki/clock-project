# Design

## Context

The firmware service already exists (`openspec/specs/ble-schedule-service`): one 113-byte read/write characteristic, open link, advertising only during a 60 s window opened by a 2 s button hold, link dropped 60 s after connect or 30 s after the last write. `web/` holds only a README. See proposal.md for motivation.

Web Bluetooth needs a secure context (HTTPS or `localhost`) and a Chromium-based browser. Firefox and Safari do not support it.

## Goals / Non-Goals

**Goals:**
- Static page, no build step, no runtime dependency, served from `localhost`.
- Encode and decode logic testable in Node without a browser or a device.
- Same wire format as the firmware, with one definition of it on the web side.

**Non-Goals:**
- Hosting or HTTPS deployment of the page.
- Pairing, bonding, notifications, setting the clock time.
- Offline/PWA features, framework, bundler, TypeScript.
- Mobile browser testing (Android Chrome should work but is not verified).

## Decisions

**Files.** `web/index.html` (markup and styles), `web/schedule.js` (ES module: `encode`, `decode`, `findDuplicates`, no DOM or Bluetooth access), `web/app.js` (DOM and Web Bluetooth glue), `web/schedule.test.mjs` (assert-based Node check, run with `node`). Alternative: one single HTML file. Rejected: the codec could not then be imported by the Node check.

**State model.** Table is `days[7][8]` of `{enabled: bool, minute: int | null}`; `null` minute means unused slot (`0x0000`). Decode maps `0x0000` to unused; any other value to `{enabled: bit15, minute: bits 0..10}`. Encode writes unused as `0x0000`. A disabled slot that keeps a time (bit 15 clear, minute set) round-trips unchanged, so the read-back compare is bytewise.

**Validation.** `findDuplicates` mirrors the firmware rule: two enabled slots with the same minute on one weekday. Minute above 1439 cannot be produced by the `<input type="time">` control, and decode rejects it as a malformed read. Upload is disabled while any duplicate exists.

**Connect.** `navigator.bluetooth.requestDevice` with filter `{name: 'ShakeClock', services: [SERVICE_UUID]}` and `optionalServices: [SERVICE_UUID]`. UUIDs (read from `firmware/src/ble_sched.c`): service `e1430001-6e9b-4b29-ba99-6f30cbe203d0`, characteristic `e1430002-6e9b-4b29-ba99-6f30cbe203d0`. The SoftDevice replaces the 16-bit slot of the registered base `e143xxxx-...` (the `e1430d6d-...` string in `ble-schedule-service` is that unreplaced base). Alternative: `acceptAllDevices`. Rejected: lists every BLE device in the chooser and needs no less configuration.

**Upload.** `writeValueWithResponse` of the 113 bytes (long write handled by the stack with MTU up to 247), then `readValue`, then bytewise compare. Write with response is required so an ATT rejection surfaces as an exception. Alternative: trust the write result. Rejected: read-back is cheap and proves flash-saved state is what the user sees.

**Disconnect handling.** `gattserverdisconnected` event sets state to disconnected, keeps the edited table, and offers Connect again. No auto-reconnect: the advertising window is closed after a connect by design, so a reconnect needs a new button hold.

**Timing.** The firmware drops an idle link after 60 s, so the page reads on connect immediately and does not rely on a long editing session while connected. The user edits first, connects, reads, and the page keeps local edits made while disconnected. On connect, the table read from the clock replaces the local table only if the local table is unmodified; otherwise the page asks which to keep. Rationale: edits must not be lost to the 60 s idle drop.

**Serving.** `python3 -m http.server` inside a `nix-shell -p python3` (or `nix-shell -p nodejs` for the test) on `localhost`. No flake change. Browser: Chromium via `nix-shell -p chromium` if none is installed.

## Risks / Trade-offs

- Browser support is Chromium only. Acceptable for a thesis demo; documented in `web/README.md`.
- Linux Web Bluetooth in Chromium may need `chrome://flags/#enable-experimental-web-platform-features` on some builds and BlueZ running. Check during the DK run; log the result in `notes/log/`.
- Open link, no pairing: anyone in range during the 60 s window can write. Accepted by the firmware spec; the window is opened by hand.
- The `local table vs clock table` prompt adds a small amount of UI. Kept because losing edits to the 60 s idle drop is the most likely user-visible failure.
- End-to-end verification needs the DK, a human button hold and a browser; only the codec is automatable.
