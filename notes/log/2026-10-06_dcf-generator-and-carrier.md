# 2026-10-06 — DCF generator and 77.5 kHz carrier

Archived changes: `add-nucleo-dcf77-generator`, `add-nucleo-dcf77-rf-carrier`.

- **Done:** Nucleo-F411RE sends DCF-77 frames, including fault frames. Pulse timing and fault keys checked on a logic analyzer. A second tool modulates a 77.5 kHz carrier on PA6. The DCF-1060N-800 demodulated that carrier. Logic-only generator kept as a fallback. Level/distance sweep marked obsolete after the over-the-air check.
- **Failed:** Real DCF-77 in the room gave no valid pulses. A commercial DCF clock in the same room also receives poorly. The logic generator never proved the receiver module itself works.
- **Fix:** Bench source for the decoder, then a real carrier for the module.
- **Why:** Decoder tests cannot wait for Frankfurt reception.
- **Evidence:** `firmware/tools/dcf77-generator/`, `firmware/README.md` bench notes, commits 2026-10-06.
