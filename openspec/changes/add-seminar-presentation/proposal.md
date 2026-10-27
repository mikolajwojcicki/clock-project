## Why

The course "Seminarium Praca Dyplomowa" requires the author to present the diploma project in about 10 minutes: what it is, why, and how far it has come. No presentation exists. The thesis tree builds only `main.pdf`, so slides would have to live outside version control and CI.

## What Changes

- Add `clock-project-latex/presentation.tex`: a Beamer deck in Polish, a second compile target next to `main.tex`. Same repo, same CI.
- Deck is 11 frames at most (title + about 10 content frames, about 1 min each), no speaker notes.
- Deck uses only real photos and measurement plots that already exist in the repo (e-paper HAT, DCF-77 module, e-paper test patterns, PPK2 captures). No placeholders.
- Copy the chosen images into `clock-project-latex/figures/prezentacja/` so the submodule builds on its own in CI.
- Extend `clock-project-latex/.github/workflows/build-thesis.yml` to compile `presentation.tex` and upload artifact `presentation-pdf` next to `thesis-pdf`.
- No local Nix shell is added; the PDF is obtained from CI (`gh run download`).
- Bump the submodule pointer in the parent repo.

## Capabilities

### New Capabilities
- `seminar-presentation`: the 10-minute Beamer introduction of the project (content, length, language, assets, build target, CI artifact).

### Modified Capabilities
- None. `thesis-document-structure` is untouched: `main.tex` and its CI artifact keep their behavior.

## Impact

- `clock-project-latex/` (separate git repo): new `presentation.tex`, new `figures/prezentacja/*`, edited CI workflow.
- Parent repo: submodule pointer bump, `notes/ai-use.md` entry for any kept AI-drafted slide text, a dated note in `notes/log/`.
- No firmware, hardware or web changes.
- Non-goals: speaker notes, English version, new photos, local TeX toolchain, WIT-official Beamer theme (none supplied), results that do not exist yet (BLE, web page, thesis prose).
- Assumption: audience is the seminar group and lecturer (mixed technical level), so motivation and photos weigh more than register-level detail.
