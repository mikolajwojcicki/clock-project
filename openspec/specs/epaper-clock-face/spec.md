# epaper-clock-face Specification

## Purpose
Keep every digit and weekday letter on the e-paper clock face readable at real size, so no digit reads as a letter or as another digit.

## Requirements

### Requirement: Digits are distinguishable
Every digit `0` to `9` on the face SHALL be recognizable as that digit at the face size, and SHALL NOT resemble a letter used on the face or another digit.

#### Scenario: Eight is not a B
- **WHEN** the digit `8` is rendered
- **THEN** its top and bottom corners are open (rounded) and it differs from the letter `B` shape

#### Scenario: All glyphs differ
- **WHEN** all digit and weekday-letter glyphs are compared pairwise
- **THEN** no two are identical

### Requirement: Weekday letters are readable
The Polish weekday abbreviations drawn on the face (`PN WT SR CZ PT SO ND`) SHALL be readable and each letter SHALL differ from every digit.

#### Scenario: O and 0
- **WHEN** `SO` and `00` are rendered
- **THEN** the `O` and the `0` are visibly different

### Requirement: Author confirms on the display
The glyph set SHALL be checked on the e-paper at real size by the author.

#### Scenario: Photo
- **WHEN** the author photographs the display showing `88:88` and a sample time
- **THEN** the photo is stored in `notes/media/` and the author confirms that no digit reads as a letter
