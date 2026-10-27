## Context

See proposal.md for motivation. Facts found while planning:

- `clock-project-latex` is its own git repo (submodule). Its CI (`.github/workflows/build-thesis.yml`) uses `xu-cheng/latex-action@v3` with `root_file: main.tex`, then uploads `main.pdf`.
- This host has no TeX (`kpsewhich` missing). The user chose CI-only builds.
- `main.tex` is `report` class, Times/`newtx`, Polish, with title macros `\stTytulPL`, `\stAutor`, `\stOpiekun`. `settings.tex` is thesis-specific (margins, chapter formatting) and not suitable for Beamer.
- Photos available (all real, none of the full perfboard):
  - `temp-resources/IMG_4276.jpeg`, `IMG_4277.jpeg`: e-paper with checkerboard test pattern.
  - `temp-resources/drive-download-20260913T135041Z-1-001/IMG_4146.jpg`: Waveshare 2.13" HAT V4 (back).
  - same folder `IMG_4149.jpg`: DCF-77 receiver module with ferrite antenna.
  - `notes/media/2026-10-17_clock_epaper-upright.jpeg`, `2026-10-17_clock_epaper-glyphs.jpg`: clock face on the e-paper.
  - `notes/media/2026-10-24_deep-sleep_ppk2-*.png`, `2026-10-25_deep-sleep_ppk2-C.jpg`: PPK2 captures.
- Milestone (from `HANDOFF.md`, `notes/log/2026-10-24_deep-sleep-current.md`): clock firmware runs on DK + breadboard; `EPD_RST` leak (2.07 mA) found, fix committed but not flashed; SPI idle-level leak (~1.8 mA) not fixed; BLE change 0/20 tasks; thesis chapters are skeletons.

## Goals / Non-Goals

**Goals**
- One Beamer file, readable in 10 minutes, built by the same CI run as the thesis.
- Slide content is sourced from notes and git history, not memory.

**Non-Goals**
- Local Nix shell or Makefile for TeX.
- Reusing `settings.tex` or the thesis bibliography.
- Speaker notes, handouts, English variant, animations.

## Decisions

1. **File location: `clock-project-latex/presentation.tex`, root of the submodule.** Same level as `main.tex`, so one `latex-action` call handles both and the Polish/UTF-8 setup is shared by convention. Alternative: a `presentation/` subdirectory. Rejected: needs extra path handling in CI and `\graphicspath` for no gain with one file.
2. **Standalone preamble, not `\input{settings}`.** `settings.tex` sets thesis page geometry and chapter styles that fight Beamer. Duplicate only what is needed: `\usepackage[T1]{fontenc}`, `inputenc` utf8, `babel` polish, `graphicx`, `booktabs`. Title strings are copied from `main.tex` and kept equal by a spec scenario.
3. **Theme: built-in `metropolis` is not in every TeX Live install used by `latex-action`; use stock Beamer `default` theme with `\usecolortheme` and custom `\setbeamercolor` for a plain blue accent, 16:9 (`aspectratio=169`), no navigation symbols.** Minimal dependency risk in CI. Alternative: metropolis (nicer, but needs the `beamertheme-metropolis` package and `fontspec`/LuaLaTeX for its fonts). Rejected: forces a different engine from the thesis.
4. **Engine: `pdflatex`.** Same as thesis; latex-action default.
5. **Images copied into `figures/prezentacja/`, downscaled JPEG/PNG.** The submodule must build alone. Phone originals are 4032x3024 (several MB); downscale to about 1600 px on the long side to keep the repo small. Source path of each copy is recorded in a comment next to its `\includegraphics`. HEIC files are not used (JPG twins exist).
6. **Frame plan (about 1 min each):**
   1. Title.
   2. Problem and goal (phone out of the bedroom, motion dismiss).
   3. Requirements (per-weekday alarms, ultra-low power, DCF-77, BLE config).
   4. Architecture block diagram (nRF52832, LIS3DH, e-paper, DCF-77, buzzer, charger). Drawn in TikZ in the file; no external image.
   5. Hardware prototype (DK + breadboard to perfboard path; e-paper HAT and DCF module photos).
   6. Low-power approach (deep sleep, interrupt wakes, power-gated DCF, e-paper only on refresh).
   7. DCF-77 sync (coding, periodic sync window, power-gated receiver).
   8. Own DCF-77 generator (see decision 10): why it was needed, the carrier trick, photo of the DCF-77 module.
   9. E-paper clock face and motion dismiss (clock-face photo, LIS3DH interrupt).
   10. Measurements (PPK2 plot, leak found: `EPD_RST` 2.07 mA, nRF alone 2 uA). Numbers copied from the dated note.
   11. Where I am and where I am headed, one frame, deliberately vague: a short "done so far" line and a short "next" line, no task lists, no dates. Marked "stan na <date>". The thesis timeline from `notes/deadlines.md` stays out (it is only assumed).
7. **Source TikZ only for the block diagram.** `tikz` ships in every TeX Live scheme latex-action uses.
8. **CI: add `presentation.tex` to the same `latex-action` step as a second `root_file` line, plus a second `upload-artifact` for `presentation.pdf`.** One job, one TeX container start. Alternative: separate job. Rejected: doubles checkout and container setup. If the multi-root form misbehaves, fall back to two sequential steps in the same job.
9. **Two commits in the submodule (`thesis:` prefix): deck + figures, then CI.** Then one parent commit bumping the pointer, `notes/ai-use.md` and the log note.
10. **Own DCF-77 generator gets its own frame.** Real DCF-77 gave no valid pulses indoors, so the decoder could not be tested. A Nucleo-F411RE emits the DCF-77 frame on `PA0` (100/200 ms pulses, operator-set time, injectable faults). To test the real receiver module too, the Nucleo drives a 77.5 kHz square wave on `PA6` into a coupling loop next to the module's ferrite antenna; during each pulse the timer duty is switched so the carrier amplitude drops to about 15 %, which is real amplitude modulation produced by pulsing one pin. The module demodulated it. Sources: `notes/log/2026-10-06_dcf-generator-and-carrier.md`, spec `dcf77-test-generator`. Pitch: clever, cheap, repeatable. Measured carrier 77492.96 Hz, -90.8 ppm against the analyzer reference (log 2026-10-17, libopencm3 port); say "about 77.5 kHz" on the slide unless the author wants the figure.

## Risks / Trade-offs

- No local build: syntax errors surface only after push. Mitigation: push the deck early, read the CI log with `gh run view --log-failed`, iterate in small commits.
- Overrun: 10 frames of dense content can exceed 10 minutes. Mitigation: at most 3 bullets per frame, photos carry content; the author rehearses once and cuts.
- Status slide goes stale (measurements and flashing happen Thursday). Mitigation: numbers come from the dated log at apply time; state "stan na <date>" on the frame.
- Beamer `babel polish` font issues in CI image: use `T1` + `lmodern`; stock behavior with `latex-action`.
- Phone photos may include unrelated surroundings or EXIF GPS data. Downscaling with `magick`/`convert -strip` removes EXIF.
- Plot screenshots from PPK2 may have a dark theme. AGENTS.md wants a light theme for evidence screenshots; if a chosen PNG is dark, replace it with a light one or crop to the data.

## Open Questions

- None blocking. The seminar date and any lecturer-specific template are unknown; if a template appears, swap the theme only (decision 3).
