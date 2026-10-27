# 2026-10-27 — seminar Beamer deck

- **Date:** 2026-10-27
- **Done:** `clock-project-latex/presentation.tex` (11 frames, Polish, no speaker notes, about 10 min) for the subject "Seminarium Praca Dyplomowa". Photos and the C5 plot are in `clock-project-latex/figures/prezentacja/`. CI builds it as artifact `presentation-pdf`. Submodule commits `8b2a8dd`, `e642626`.
- **Failed:** nothing compiled yet. No local TeX on this host, and `git push` is blocked for the agent, so the first compile is the CI run after the author pushes.
- **Fix:** —
- **Why this part or library:** stock Beamer theme, because `metropolis` needs LuaLaTeX and extra packages in the CI image. CI-only build by the author's choice.
- **Evidence:** C5 plot is window 380-545 s of `notes/measurements/ppk-20261025T210658.ppkz`; numbers from `notes/log/2026-10-24_deep-sleep-current.md` (2.06 mA idle, about 2 µA after `EPD_RST` off).
- **Open:** push, read CI log, render pages and check diacritics and overflow (OpenSpec tasks 4.1-4.3).
