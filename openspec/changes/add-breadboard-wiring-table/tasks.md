## 1. Table

- [x] 1.1 Create `hardware/docs/breadboard-wiring-table.md` from the draft in `design.md`; check each row against guide sections 6 and 9 onward and `firmware/boards/dk_breadboard_pins.h`
- [x] 1.2 Add the rebuild order (GND and `3V3`, SPI bus, signals, buzzer stage) with a continuity check after each group and a `3V3`-to-`GND` short check
- [x] 1.3 Add one link line to the table in guide section 6

## 2. Verify

- [ ] 2.1 Rebuild the board from the table; run `dcf sync` and one display refresh to confirm nothing changed; note wire mistakes found
- [ ] 2.2 Commit: `hardware: add per-wire breadboard connection table`
