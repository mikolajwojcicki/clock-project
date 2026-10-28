# Design

## Context

Today `poll_clock` sets `s_draw` whenever `s_time.min != s_drawn_min`, and the
main loop draws when `s_draw` is set, not drawing and no DCF window is open
(`firmware/apps/clock/src/main.c`). `s_draw` is also set on UART set and DCF
accept. Buttons already wake the CPU (`board_button_wake_enable`, PORT/SENSE)
and `board_any_button_pressed()` is polled in `poll_alarm` only to silence the
alarm. Motivation and numbers: see proposal.md.

## Goals / Non-Goals

**Goals:**
- Refresh only on the events in `display-refresh-policy`.
- Keep alarm timing, button silence and DCF window behavior unchanged.

**Non-Goals:**
- Partial refresh, panel or DCF rail gating, explaining the 16.7 vs 8 mC gap.
- BLE or web changes.

## Decisions

- **Remove the minute trigger, keep `s_draw` as the single request flag.**
  The loop, DCF-window deferral and `s_drawing` guards stay. Alternative: a
  second "button" flag. Rejected: same effect, more state.
- **Detect a press as a released-to-pressed edge in `app_poll`.** The pin is
  polled after each PORT wake; a `was_pressed` latch gives one request per
  press, so a held button and a press during a refresh do not repeat.
  Alternative: use the GPIOTE handler `on_button`. Rejected: it runs in
  interrupt context and gives no release information without extra pin reads.
- **A press that silences the alarm does not request a refresh, and an alarm
  stop does not either.** The author chose to save the refresh; the user
  presses again to see the time. Alternative: refresh on stop. Rejected: one
  more 16.7 mC refresh per alarm for a face nobody reads right away.
- **Button silence is a prototype feature.** The final firmware is expected to
  stop the alarm by motion only, so button handling stays minimal here and no
  requirement depends on it. Removing it is a separate change.
- **Stale face is accepted.** The face shows the time of its last refresh.
  The seconds counter and RTC keep running; only the panel image is old.

## Risks / Trade-offs

- Press-to-image latency is about 3.7 s (C9) → accept for a bedside check; a
  shorter partial refresh is a separate change.
- A glance at a stale face shows a wrong time with no sign → a press always
  fixes it; consider a stale marker later, not in this change.
- Press events lost while a DCF window is open → the request flag stays set and
  the refresh runs when the window closes (at most 5 minutes).
- Daily energy depends on how often the user presses → the policy run logs
  the assumption (20 presses a day is `[INFERENCE]`).
