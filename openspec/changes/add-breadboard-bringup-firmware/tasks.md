# Tasks

## 1. App skeleton, safe states, console

- [x] 1.1 Create `firmware/boards/dk_breadboard_pins.h` with every signal from guide section 6, DK buttons 1 to 4, and the DCF active-level and pull constants; verify by diffing each pin against the guide table
- [x] 1.2 Create `firmware/apps/bringup/` from blinky (wrapper `Makefile`, `armgcc/Makefile`, linker script, `config/sdk_config.h`) with project name `bringup_pca10040` and `firmware/src/` on the source and include paths; verify `make` produces `armgcc/_build/nrf52832_xxaa.hex`
- [ ] 1.3 Add `board_safe_state()` in `firmware/src/` and call it as the first statement of `main()`; verify with a multimeter or logic analyzer after `make flash`: `P0.24`, `P0.11`, `P0.30` high and `P0.31` low
- [ ] 1.4 Add a polled UART0 console at 115200 baud and print the banner with app name, build identifier, button map, and `RESETREAS`; verify the banner appears in `picocom -b 115200 /dev/ttyACM0` after a reset
- [ ] 1.5 Start LFCLK from the crystal and RTC1 as a free-running timebase; add button handling through GPIOTE with 50 ms debounce and the one-test-at-a-time rule; verify each button prints its test name and a second button during a stub test prints "test already running"
- [x] 1.6 Add `firmware/apps/bringup/README.md` (build, flash, erase, serial console, button map, erased-chip floating-pin warning) and update `firmware/README.md` layout; verify the documented commands run as written
- [ ] 1.7 Add VS Code tasks `firmware: bringup build`, `firmware: bringup flash`, `firmware: serial console`; verify each task runs from Terminal > Run Task
- [ ] 1.8 Commit: `firmware: add breadboard bring-up app with safe pin states`

## 2. Shared SPI bus and e-paper test

- [ ] 2.1 Implement the shared SPIM0 bus driver (1 MHz, per-device SPI mode, one CS asserted per transfer, both CS high between transfers); verify with the logic analyzer that only one CS goes low per transfer
- [ ] 2.2 Implement the Waveshare 2.13" V4 driver: reset, busy wait with 10 s timeout, full refresh of a generated border + checkerboard frame, deep sleep; verify on hardware that the pattern appears and stays after USB is removed
- [ ] 2.3 Wire button 1 to the display test with `PASS` / `FAIL` reporting and `board_safe_state()` on every exit; verify a busy timeout (display `BUSY` wire removed, USB off while rewiring) reports `FAIL` with the reason
- [ ] 2.4 Document the display test, expected pattern, and panel-revision check in the bring-up README; verify against guide section 9.2
- [ ] 2.5 Commit: `firmware: add shared SPI bus and e-paper test pattern`

## 3. LIS3DH test

- [ ] 3.1 Implement LIS3DH `WHO_AM_I` read and report; verify `0x33` and `PASS` on the wired sensor, and `FAIL` with the read value when sensor `CS` is disconnected (USB off while rewiring)
- [ ] 3.2 Configure the high-pass `INT1` motion interrupt (100 Hz, about 250 mg, latched) and report each event with `INT1_SRC` axes until button 2 or 60 s timeout; verify events appear while moving and stop when still
- [ ] 3.3 Document the LIS3DH test in the bring-up README; verify against guide section 10.2
- [ ] 3.4 Commit: `firmware: add LIS3DH identify and INT1 motion test`

## 4. DCF-77 test

- [ ] 4.1 Implement the pure pulse classifier in `firmware/src/dcf77_classify.c` and the host check `firmware/tests/dcf77_classify_test.c`; verify `gcc -I firmware/src firmware/tests/dcf77_classify_test.c firmware/src/dcf77_classify.c -o /tmp/dcf_test && /tmp/dcf_test` exits 0 (use `nix-shell -p gcc` if host `gcc` is missing)
- [ ] 4.2 Implement the DCF test: `DCF_PON` low only during the window, edge timestamps from RTC1, per-pulse log lines, stop on button 3 or 10 min timeout with counts; verify with a multimeter that `P0.24` is low only while the test runs
- [ ] 4.3 Verify on hardware with `OUT` connected after its level check: about one pulse per second is logged, or zero valid pulses is reported as an observation
- [ ] 4.4 Document the DCF test, the floating-`OUT` warning, and the pull-up constant in the bring-up README; verify against guide section 11.3
- [ ] 4.5 Commit: `firmware: add DCF-77 pulse capture and classifier`

## 5. Buzzer test

- [ ] 5.1 Implement the two-step buzzer test (500 ms steady high, then 500 ms 2.7 kHz tone via PWM0) with a report before each step and PWM stopped in `board_safe_state()`; verify with the logic analyzer that `P0.31` is low before, between, and after the steps
- [ ] 5.2 Verify on hardware that the buzzer sounds in at least one step and the DK does not reset; record which step sounds
- [ ] 5.3 Document the buzzer test and how to record the drive type in the bring-up README; verify against guide section 12.3
- [ ] 5.4 Commit: `firmware: add bounded buzzer drive test`

## 6. Integration

- [ ] 6.1 With all modules wired per guide section 13 step 6, run tests 1 to 4 in order after one reset; verify each ends with its expected result line and the safe states hold between tests
- [ ] 6.2 Run `openspec validate add-breadboard-bringup-firmware --strict`; verify it passes
