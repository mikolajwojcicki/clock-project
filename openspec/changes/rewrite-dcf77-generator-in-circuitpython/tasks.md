## 1. Feasibility gate

- [ ] 1.1 Ask the author which board is on hand and whether a USB breakout and a source build are acceptable; choose option A, B or C from design decision 1
- [ ] 1.2 For option A or B, check that `pwmio` can output 77.5 kHz on the chosen pin and measure the real frequency on Saleae
- [ ] 1.3 Write the decision and evidence in `notes/log/`; if no-go, stop here and leave the C generator in place

## 2. Port (only after go)

- [ ] 2.1 Install CircuitPython on the board using a `nix-shell` with the DFU tool; record the exact commands in the README
- [ ] 2.2 Port `dcf77_encode` to Python with a host check that matches the C encoder for fixed times
- [ ] 2.3 Port the pulse scheduler, UART/console commands, fault keys and carrier keys with the same pins and levels
- [ ] 2.4 Update the README for the Python generator

## 3. Verify

- [ ] 3.1 Saleae 190 s at 1 MS/s: pulse widths, spacing, minute gap and decoded frames against the 2026-10-06 results
- [ ] 3.2 Carrier frequency, duty levels and dip lag against the 2026-10-06 table
- [ ] 3.3 DK bring-up DCF test for 160 s: valid pulses, minute markers, decoded frames equal the log
- [ ] 3.4 Receiver test with DCF-1060N-800: still locks and decodes consecutive minutes (author, on request)

## 4. Cleanup and record

- [ ] 4.1 Remove the C generator in its own commit once 3.1 to 3.3 pass
- [ ] 4.2 Add a dated note in `notes/log/` with results, deviations and the firmware commit
