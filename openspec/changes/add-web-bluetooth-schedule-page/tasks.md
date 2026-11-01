## 1. Codec

- [x] 1.1 Read the characteristic UUID from the firmware source and record service and characteristic UUIDs as constants in `web/schedule.js`
- [x] 1.2 Write `encode`, `decode` and `findDuplicates` in `web/schedule.js` (no DOM, no Bluetooth); `decode` rejects wrong length, version not 1 and minute above 1439
- [x] 1.3 Write `web/schedule.test.mjs` with asserts: Monday slot 1 07:00 enabled gives `01 A4 81`, unused slot is `00 00`, encode/decode round trip including a disabled slot that keeps a time, same-day duplicate found, same time on two days not flagged, bad length/version/minute rejected
- [x] 1.4 Run the test with `nix-shell -p nodejs --run "node web/schedule.test.mjs"` and see it pass
- [x] 1.5 Commit codec and test (`web:`)

## 2. Page

- [ ] 2.1 Write `web/index.html` and `web/app.js`: 7 days by 8 slots, enable toggle and time input per slot, modified marker, duplicate marking, Upload disabled on duplicates
- [ ] 2.2 Add Connect with the `ShakeClock` + service filter, connection state, hint to hold a button for 2 s when no device, unsupported-browser message that disables Connect
- [ ] 2.3 On connect read and decode the table; keep local edits when the local table is modified and ask which to keep
- [ ] 2.4 Upload with write-with-response, read back, compare bytewise, show success or error; keep the edited table on error; handle `gattserverdisconnected` without auto-reconnect
- [ ] 2.5 Update `web/README.md`: how to serve on localhost, supported browsers, how to open the advertising window
- [ ] 2.6 Commit the page (`web:`)

## 3. Verify on the DK

- [ ] 3.1 Serve `web/` with `nix-shell -p python3 --run "python3 -m http.server 8000 --directory web"` and open it in Chromium on `localhost`; record any browser flag needed
- [ ] 3.2 Start a UART console capture first, confirm it runs, then hold a DK button 2 s and connect from the page; the page shows connected and the table read from the clock
- [ ] 3.3 Set Monday slot 1 to a time a few minutes ahead, Upload; the page shows success and the console prints `ble: write ok`
- [ ] 3.4 Let the alarm fire at that time; the buzzer starts and the console prints `ALARM`
- [ ] 3.5 Create a same-day duplicate; the page marks both slots and Upload is disabled
- [ ] 3.6 Connect, wait for the idle drop, confirm the page shows disconnected and keeps the edits
- [ ] 3.7 Write `notes/log/2026-11-01_web-bluetooth-page.md` with the evidence (console excerpts, browser version, flags) and commit (`web:`)
