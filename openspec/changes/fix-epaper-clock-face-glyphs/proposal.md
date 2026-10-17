## Why

On the e-paper face some digits look like letters. The `8` bitmap in `firmware/src/clock_face.c` is `1E 11 11 1E 11 11 1E`, which is the letter `B` (flat top corners). The author reports more such cases. The face uses a 5x7 bitmap font scaled by 3 and 4, and several shapes were drawn with square corners.

Observed in `k_digit[]`: `8` equals a `B` shape; `9`, `6`, `2`, `3`, `5` have the same flat top and bottom corners; `4` is drawn as two verticals joined by a bar, with no diagonal. Letters (`P N W T S R C Z D O`) and the colon were not reviewed yet.

## What Changes

- Redraw all ten digits so each reads clearly at the face sizes, with rounded corners.
- Review the weekday letters the same way and fix any that clash with a digit or with each other.
- Add a host check that renders every glyph to text, so shapes can be reviewed without the board, and checks that all glyphs are pairwise different.
- Check on the e-paper at real size; the author takes the photo.

## Capabilities

### New Capabilities
- `epaper-clock-face`: legibility of digits, weekday letters and colon on the face.

### Modified Capabilities
<!-- none -->

## Impact

- `firmware/src/clock_face.c`, a new host check under `firmware/tests/`, `firmware/README.md` for the run line, `notes/media/` for the photo.
- Does not touch layout, scale or refresh logic.
- Locked architecture unchanged.
