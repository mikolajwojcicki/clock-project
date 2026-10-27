## 1. Assets

- [x] 1.1 Create `clock-project-latex/figures/prezentacja/`. Copy and downscale (about 1600 px long side, `-strip` for EXIF) the photos listed in design.md: e-paper HAT back, DCF-77 module, e-paper test pattern, clock face (upright + glyphs).
- [x] 1.2 Pick one PPK2 capture for the measurement frame from `notes/media/`; if it is dark-themed, crop or replace with a light one. Copy it into `figures/prezentacja/`.
- [x] 1.3 Commit assets in the submodule (`thesis:` prefix); body names each source file and why it was picked.

## 2. Deck

- [x] 2.1 Create `clock-project-latex/presentation.tex`: standalone Beamer preamble (16:9, pdflatex, T1, utf8, polish), plain blue theme, no navigation symbols, title frame with `\stTytulPL`, `\stAutor`, `\stOpiekun`, programme copied from `main.tex`.
- [x] 2.2 Write frames 2-4 (problem and goal, requirements, TikZ architecture block diagram).
- [x] 2.3 Write frames 5-9 (hardware prototype with photos, low-power approach, DCF-77 sync, own DCF-77 generator and carrier trick from `notes/log/2026-10-06_dcf-generator-and-carrier.md` and spec `dcf77-test-generator`, e-paper clock face and motion dismiss).
- [x] 2.4 Write frames 10-11 (measurements with run names and numbers read from `notes/log/2026-10-24_deep-sleep-current.md`; one brief closing frame combining what is done and where the work is headed, general wording, "stan na" date).
- [x] 2.5 Check: at most 3 bullets per frame, no speaker notes, no placeholders, every `\includegraphics` path under `figures/prezentacja/`, frame count 11 or fewer.
- [x] 2.6 Commit the deck in the submodule (`thesis:` prefix) with a body stating audience (Seminarium Praca Dyplomowa) and the frame plan.

## 3. CI

- [x] 3.1 Edit `clock-project-latex/.github/workflows/build-thesis.yml`: add `presentation.tex` as a second `root_file`; add an `upload-artifact` step `presentation-pdf` for `presentation.pdf` (`if-no-files-found: error`). Keep `thesis-pdf`.
- [x] 3.2 Commit the workflow change separately in the submodule (`ci:` prefix, matching the existing log).

## 4. Verify

- [x] 4.1 Push the submodule; wait for the CI run (`gh run watch`), fix failures from `gh run view --log-failed`, repeat.
- [x] 4.2 Download both artifacts with `gh run download -R mikolajwojcicki/clock-project-latex -n presentation-pdf` and `-n thesis-pdf`; confirm `presentation.pdf` has 11 or fewer pages and `main.pdf` still builds.
- [x] 4.3 Render pages to PNG (pdf_render_pages) and check: Polish diacritics, photos visible, no overflow, readable at projector size.

## 5. Parent repo

- [x] 5.1 Add `notes/log/2026-10-27_seminar-presentation.md` from `notes/TEMPLATE.md` (what was done, what failed, why Beamer stock theme, CI-only build).
- [x] 5.2 Add a `notes/ai-use.md` entry for the AI-drafted slide text; remind the author to rewrite it in their own words before presenting.
- [ ] 5.3 Commit the submodule pointer bump (`thesis:` prefix), then the notes as a separate commit.
