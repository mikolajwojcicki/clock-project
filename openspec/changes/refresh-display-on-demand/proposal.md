# Proposal

## Why

Run C9 (`c6faefd`, panel on) measured 16.75 mC per refresh. A refresh every
minute is about 6.7 mAh/day, 79 % of the ~8.5 mAh/day rail budget, while the
device is an alarm clock that is read only when the user looks at it. Idle is
only 1.8 mAh/day. Refreshing on demand removes most of the energy cost.

## Changes

- Stop the automatic refresh on every minute change.
- Refresh the face when the user presses a button and no alarm is sounding.
- Refresh the face when an alarm starts. No refresh when it stops.
- Keep refreshing after the time is set (UART) or corrected by DCF-77, so the
  face never shows a time the device has just replaced.
- The face shows the time of its last refresh. Between refreshes it can be
  stale by design.
- Record the daily energy of the new policy from a measured run, replacing
  the `[INFERENCE]` of about 20 refreshes a day in
  `notes/log/2026-10-28_deep-sleep-current.md`.

## Capabilities

### New Capabilities

- `display-refresh-policy`: when the firmware refreshes the e-paper face
  (button, alarm start, time set), and when it does not.

### Modified Capabilities

- `idle-pin-states`: requirement "Peripheral traffic is unchanged" says the
  panel refreshes every minute; it changes to the new policy.
- `firmware-sdk-drivers`: requirement "Observable behavior is unchanged" has a
  scenario with a refresh each minute; it changes to the new policy.

## Impact

- `firmware/apps/clock/src/main.c`: `poll_clock` draw trigger
  (`s_time.min != s_drawn_min`), `poll_alarm`, and a button-press path.
- `firmware/src/board.c`: button wake already exists
  (`board_button_wake_enable`); the press must be reported once per press.
- `notes/log/`: a new measurement run with the policy, and a corrected daily
  energy table.
- Not in scope: the 16.7 mC vs 8 mC refresh charge gap, the 72 µA idle gap on
  the shared rail, partial refresh, power-gating the panel or DCF rail.
