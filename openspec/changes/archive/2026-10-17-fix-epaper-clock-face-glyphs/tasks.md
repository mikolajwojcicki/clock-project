## 1. Glyphs

- [x] 1.1 Replace `k_digit[]` with the redrawn digits from design decision 2; adjust after the render if any shape is still unclear
- [x] 1.2 Review `letter()` glyphs (`P N W T S R C Z D O`) and the colon; fix any clash with a digit or another letter; use a slashed `0`

## 2. Host check

- [x] 2.1 Add a glyph accessor in `clock_face.[ch]` and `firmware/tests/clock_face_test.c`: print each glyph as `#` / `.` and assert pairwise inequality
- [x] 2.2 Add the run line to the host-check section of `firmware/README.md`
- [x] 2.3 Run the host check inside a `nix-shell` that provides `gcc` and review the printed shapes

## 3. Verify on the display

- [x] 3.1 Build with `make -C firmware/apps/clock` without warnings and flash
- [x] 3.2 Show `88:88`-like times and every weekday on the e-paper; author reviews each digit and letter
- [x] 3.3 Author photographs the display into `notes/media/` as `YYYY-MM-DD_clock_epaper-glyphs.jpg`

## 4. Record

- [x] 4.1 Add a dated note in `notes/log/` with the before / after shapes, the photo name and the firmware commit
