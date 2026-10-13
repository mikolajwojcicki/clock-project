# 2026-10-04 — DK pins and bring-up firmware

Archived changes: `fix-dk-power-and-pin-map`, `harden-prototype-safety-guide`, `add-breadboard-bringup-firmware` (archived 2026-10-06; the firmware landed this day).

- **Done:** Pin map moved off DK buttons, LEDs, and the reset pin. Bring-up app drives safe states first, then display, LIS3DH, DCF capture, and a short buzzer pulse. Verified on the DK: UART banner, buttons, PON window, LIS3DH ID and INT1, Waveshare 2.13" V4 test pattern, buzzer both steps.
- **Failed:** Guide said "DK 3V3", which is not a pin name. SPI sat on button pins. LIS3DH chip-select sat on the pin used as reset. SPI at a higher clock was noisy. LIS3DH interface needed a boot reset or the bus stayed noisy. Indoor DCF run produced no valid frame (logged 2026-10-06).
- **Fix:** Use DK VDD for 3.3 V. Remap GPIO. SPI at 250 kHz. Reset the LIS3DH interface at boot. Power the active buzzer from DK 5 V.
- **Why:** A wrong pin either fights the DK buttons or holds the chip in reset.
- **Evidence:** `firmware/apps/bringup/`, commits 2026-10-04.
