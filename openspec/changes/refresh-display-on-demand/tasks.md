# Tasks

## 1. Firmware policy

- [ ] 1.1 In `firmware/apps/clock/src/main.c`, remove the minute-change draw request and add a press-edge request that is skipped while an alarm is sounding; verify the UART build prints no `display ok` over 10 idle minutes and one per press
- [ ] 1.2 Keep the refresh at alarm start and add none at stop (button, motion, timeout); verify one `display ok` per start, none per stop, and the buzzer still shows 1 s on / 1 s off on `D2`
- [ ] 1.3 Verify held button gives one refresh, a press during a refresh gives none queued, and a press inside a DCF window refreshes after it closes (UART log)
- [ ] 1.4 Build both variants (`make` and `make SLEEP_UART=0`) and verify both link

## 2. Measurement and notes

- [ ] 2.1 Flash `SLEEP_UART=0`, record 10 min on configuration C with a press every 60 s, pack with `ppk-archive.sh pack`, and verify the pack with its read-back check
- [ ] 2.2 Log the run in `notes/log/` with idle current, charge per refresh, and the daily energy for the assumed presses per day; verify the table replaces the `[INFERENCE]` in the C9 section
- [ ] 2.3 Add the entry to `notes/ai-use.md` if any prose is kept, and commit firmware and notes separately

## Workflow follow-up

- Archive the change after the measurement note is committed.
