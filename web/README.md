# Web

Static Web Bluetooth page that reads, edits and uploads the 56-slot alarm table of the ShakeClock. No build step, no dependency.

Files: `index.html` (markup), `app.js` (DOM and Bluetooth), `schedule.js` (codec, UUIDs, duplicate check), `schedule.test.mjs` (Node check of the codec). Wire format and GATT layout: `openspec/specs/ble-schedule-service/spec.md`.

## Run

Web Bluetooth needs `localhost` or HTTPS. Serve the folder from the repo root:

```sh
nix-shell -p python3 --run "python3 -m http.server 8000 --directory web"
```

Open `http://localhost:8000` in Chrome, Chromium or Edge. Firefox and Safari have no Web Bluetooth. On Linux, BlueZ must be running; some Chromium builds also need `chrome://flags/#enable-experimental-web-platform-features`.

## Use

1. Edit the table (works offline). Edits are marked as modified.
2. Open the advertising window on the clock: hold a button for 2 s. It stays open 60 s.
3. Press **Connect**, pick `ShakeClock`. The page loads the clock table. If you have unsaved edits it asks which to keep.
4. Press **Upload**. The page writes 113 bytes, reads them back and shows success only if they match.

The clock drops the link 60 s after connect or 30 s after the last write, and does not re-advertise. After a drop the page keeps your edits; open a new window and connect again. Upload is disabled while two enabled slots on one day share a time. The page never sets the clock time.

## Check the codec

```sh
nix-shell -p nodejs --run "node web/schedule.test.mjs"
```
