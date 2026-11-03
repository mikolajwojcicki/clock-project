# Waveshare 2.13inch e-Paper HAT — 40-pin header research

**Target hardware:** Waveshare 2.13inch e-Paper HAT, driver PCB **Rev2.1** (sheet has no revision label; the driver sheet in the images is V2.2), 8-pin PH2.0 side cable (no PWR wire), panel V3/V4 independent of driver rev.

**Use case:** Plug the HAT’s 2×20 female onto a 2×20 male on a ~7×9 cm perfboard shield driven by an **nRF52 DK (3.3 V only)** — not a Raspberry Pi.

---

## 1. Signal → RPi physical pin map

| signal | RPi physical pin | BCM GPIO | source URL |
| --- | --- | --- | --- |
| VCC (8-pin cable / host supply) | **not** 1/17 on the 40-pin path — see §3–4; 8-pin VCC = same net as phys **2 & 4** (`RPI_5V`) | — (power) | [wiki pinout](https://www.waveshare.com/wiki/2.13inch_e-Paper_HAT_Manual); [schematic PDF](https://files.waveshare.com/upload/5/57/2.13inch_e-Paper_HAT_Schematic.pdf) |
| GND | **6, 9, 14, 20, 25, 30, 34, 39** (all bridged) | GND | [schematic PDF](https://files.waveshare.com/upload/5/57/2.13inch_e-Paper_HAT_Schematic.pdf) (`NLGND`) |
| DIN (MOSI) | **19** | **10** | [wiki](https://www.waveshare.com/wiki/2.13inch_e-Paper_HAT_Manual); [epdconfig.py](https://github.com/waveshareteam/e-Paper/blob/master/RaspberryPi_JetsonNano/python/lib/waveshare_epd/epdconfig.py); [DEV_Config.c](https://github.com/waveshareteam/e-Paper/blob/master/RaspberryPi_JetsonNano/c/lib/Config/DEV_Config.c); schematic `P_MOSI` |
| CLK (SCLK) | **23** | **11** | same as DIN |
| CS (CE0) | **24** | **8** | same as DIN |
| DC | **22** | **25** | same as DIN |
| RST | **11** | **17** | same as DIN |
| BUSY | **18** | **24** | same as DIN |

### Expected values — confirm / refute

| Expected | Result |
| --- | --- |
| VCC = 1 or 17 (3.3 V) | **Refuted for the 40-pin power path.** Pins **1 and 17 are bridged only to each other** (`P_3V3`); they are **not** wired into the HAT power rail. Main header power is phys **2 & 4** (`RPI_5V`), which is also the 8-pin **VCC** net. Wiki/demo “VCC → 3.3V” refers to the **8-pin cable** wiring convention (and matching logic level), not “feed pin 1/17 on the HAT”. |
| GND = 6/9/… | **Confirmed** — all eight GND pins bridged. |
| DIN=19, CLK=23, CS=24, DC=22, RST=11, BUSY=18 | **Confirmed** (wiki + demo BCM defines + schematic host nets through TXS0108E). |

Demo BCM pin defines (Raspberry Pi path):

```text
RST=17  DC=25  CS=8  BUSY=24  MOSI=10  SCLK=11  (PWR=18 — see §2)
```

Sources: [epdconfig.py](https://raw.githubusercontent.com/waveshareteam/e-Paper/master/RaspberryPi_JetsonNano/python/lib/waveshare_epd/epdconfig.py), [DEV_Config.c](https://raw.githubusercontent.com/waveshareteam/e-Paper/master/RaspberryPi_JetsonNano/c/lib/Config/DEV_Config.c) (`DEV_GPIO_Init`).

---

## 2. PWR pin (GPIO18 / phys 12) — Rev2.1 vs later

**Rev2.1 integrated 2.13inch e-Paper HAT (this board): does not use PWR.**

- Wiki RPi pin table for this HAT lists only the classic 8 signals (no PWR): [2.13inch e-Paper HAT Manual](https://www.waveshare.com/wiki/2.13inch_e-Paper_HAT_Manual).
- Cable is PH2.0 **8-pin** (user’s unit; matches wiki).
- Schematic sheet (read from image): phys **12** is labelled `P18` and wired only to the header; no tie into the power-switch / LDO / TXS circuitry.
- On Rev2.1, **RST doubles as power control**: RST drives `VCC_EN` → Q32 (S8050) → Q31 (AO3401) to switch `RPI_5V` → `VCC_INT` → panel rail. Wiki FAQ: holding RST low too long **power-gates the board** and breaks reset ([wiki FAQ “BUSY always busy”](https://www.waveshare.com/wiki/2.13inch_e-Paper_HAT_Manual)). C readme: after `DEV_Module_Exit()` on Rev2.1, module current is “basically 0” ([readme_EN.txt](https://raw.githubusercontent.com/waveshareteam/e-Paper/master/RaspberryPi_JetsonNano/c/readme_EN.txt)).

**Which revision added PWR?** Not verified. An earlier web index claimed the separate E-Paper Driver HAT Rev2.3 (9-pin GH1.25) adds PWR → BCM 18 / pin 12. The driver schematic image read this session is V2.2 and shows no PWR net. Treat the Rev2.3 claim as unconfirmed.

**Panel “V3 / V4”** are **display** revisions (controller/LUT/fast refresh), **not** the driver-PCB Rev2.1 silk. They do not add a PWR header pin on this HAT ([wiki Overview](https://www.waveshare.com/wiki/2.13inch_e-Paper_HAT_Manual)).

**If PWR is left unconnected:**

| Board | Effect |
| --- | --- |
| This Rev2.1 HAT | **None** — pin 12 is floating on the PCB. Demo code may still toggle BCM18; harmless here. |
| Driver HAT Rev2.3 (unverified this session) | Display stays off unless PWR is driven high / tied to 3.3 V, or the empty rear pad is fitted with ~47 kΩ ([Driver HAT wiki](https://www.waveshare.com/wiki/E-Paper_Driver_HAT)). |

**Conflict:** Unified Waveshare demos always define `PWR_PIN = 18` and drive it in `module_init` / `DEV_GPIO_Init`, even though this HAT’s wiki table and 8-pin cable omit PWR. Treat demo `PWR` as for newer/9-pin Driver HATs; do **not** require phys 12 on the nRF52 shield for Rev2.1.

---

## 3. Which 3V3 / GND / 5V header pins are actually wired?

From Altium connectivity embedded in [2.13inch_e-Paper_HAT_Schematic.pdf](https://files.waveshare.com/upload/5/57/2.13inch_e-Paper_HAT_Schematic.pdf):

| Net | 40-pin physical pins | Also | Notes |
| --- | --- | --- | --- |
| `P_3V3` | **1, 17** only | — | Bridged to each other; **no** other components on this net → **not** the HAT supply input. |
| `RPI_5V` | **2, 4** | 8-pin **VCC** (P3 pin 1) | **Main power input** into AO3401 / `VCC_INT` / LDO / TXS `VccB` path. |
| `GND` | **6, 9, 14, 20, 25, 30, 34, 39** | 8-pin GND | All bridged. |
| Signal nets | 11, 18, 19, 22, 23, 24 | matching 8-pin pins | Through TXS0108E. |

**Are 5 V pins (2, 4) connected? Yes.** Header 2/4 (`RPI_5V`) and 8-pin P3 pin 1 (`VCC_INT`) are the same pre-switch supply input (sheet: `RPI_5V`/`VCC_INT` labels at P1 bottom; P3 pin 1 at the Q31 input). Path: `VCC_INT` → Q31 (AO3401, gated by RST/`VCC_EN`) → `VCC` → RT9193-33 VIN → 3V3 rail; `VCC` also feeds TXS0108E VccB.

- Bench evidence (user, this session): panel works from the 8-pin cable with VCC at 3.3 V. Same input net, so 3.3 V on 2/4 should behave the same.
- Not verified: RT9193-33 dropout margin at 3.3 V in, Q31 Vgs at 3.3 V gate drive, header contact resistance. If the shield is reproduced, measure the 3V3 rail under refresh.
- Pins 1/17 stay isolated (bridged only to each other); do not use them for power.

Independent confirmation that the 40-pin path takes power from the Pi **5 V** pins (while docs say cable VCC → 3.3 V): [Martin Hubáček — Waveshare e-Paper HAT issues](http://martinhubacek.cz/blog/waveshare-epaper-hat-issues/).

---

## 4. 3.3 V logic only? Does the HAT need 5 V from pin 2/4?

**Panel** logic is 3.3 V (e-Paper VCI/VDDIO). **Rev2.1 driver** adds:

- **TXS0108E** bidirectional level translator (`VccA` = panel `3V3`, `VccB` = host `VCC`)
- **RT9193-33** LDO for the panel 3.3 V rail
- **AO3401** high-side switch from `RPI_5V` → `VCC_INT`, gated by RST/`VCC_EN`

Citations: [schematic](https://files.waveshare.com/upload/5/57/2.13inch_e-Paper_HAT_Schematic.pdf); wiki precaution 7 — “driver board (V2.1 and subsequent) … level processing circuit … 3.3V and 5V” ([manual](https://www.waveshare.com/wiki/2.13inch_e-Paper_HAT_Manual)).

**Does it *require* 5 V from pin 2/4?**

- Schematic: LDO VIN comes from `VCC` (= `RPI_5V` through Q31). RT9193-33 dropout not checked.
- Wiki says VCC → 3.3 V on the 8-pin cable; Arduino tables use 5 V because the MCU is 5 V.
- Bench: 3.3 V on VCC works on the 8-pin cable (same net as 2/4). Dropout/Q31 margins still unmeasured.

**Conflict (flagged):**

| Source | Claim |
| --- | --- |
| Wiki Parameters | “Operating voltage: 3.3V / 5V (**5V is required** for power supply and signal)” |
| Wiki RPi 8-pin table | VCC → **3.3V** |
| Schematic + Martin | 40-pin power = **pins 2/4** (`RPI_5V`); feeding 5 V there makes TXS host-side ~5 V (bad for 3.3 V MCUs) |
| Product blurb | “Onboard voltage translator, compatible with 3.3V / 5V MCUs” |

**Practical rule for nRF52:** signal levels are 3.3 V (TXS VccA = 3V3 from the HAT LDO). Power on 2/4 at 3.3 V (§3, bench-verified on the 8-pin cable). Never put 5 V on the shield.

---

## 5. Other header pins to leave floating

| Phys | Name | On this Rev2.1 HAT |
| --- | --- | --- |
| 12 | GPIO18 (labelled `P18` on sheet) | **No connection** on schematic — leave floating |
| 27, 28 | ID_SD / ID_SC | Labelled on sheet; **no EEPROM symbol** on the schematic (unlike [2.13inch e-Paper HAT+](https://files.waveshare.com/wiki/2.13inch-%20e-Paper-HAT%2B/2.13inch_e-Paper_HAT%2B-SCH.pdf) with CAT24C32). Leave floating. |
| 3, 5, 7, 8, 10, 13, 15, 16, 21, 26, 29, 31–33, 35–38, 40 | misc GPIO / UART / I2C / SPI1 | No loads on schematic beyond the connector — leave floating |
| 1, 17 | 3V3 | Bridged only to each other — may leave floating **or** tie to 3.3 V for mechanical completeness; **do not rely on them for HAT power** |

Shield should only hard-wire: **3.3 V into 2 and/or 4** (same net as 8-pin VCC), **all needed GNDs**, and **11, 18, 19, 22, 23, 24**.

---

## 6. Mechanical outline / header placement

| Item | Value | Source |
| --- | --- | --- |
| Driver board outline | **65 mm × 30.2 mm** | [wiki Parameters](https://www.waveshare.com/wiki/2.13inch_e-Paper_HAT_Manual); [product page](https://www.waveshare.com/product/raspberry-pi/2.13inch-e-paper-hat.htm) |
| Active display area | 48.55 mm × 23.71 mm | same |
| Raw panel outline | 59.2 mm × 29.2 mm × ~1.05 mm | same |
| Mounting holes | 3.0 mm (Driver HAT family; same board size) | [E-Paper Driver HAT](https://www.waveshare.com/wiki/E-Paper_Driver_HAT) |

**Fit on ~70×90 mm perfboard:** the HAT is essentially a **Pi Zero–sized** rectangle (65×30.2). It covers only a strip of the shield, not the full 7×9 cm.

**Header / FPC geometry (qualitative — mm offsets not verified from a readable drawing):**

- Standard Raspberry Pi **2×20** female on the PCB bottom; pin 1 mates with Pi pin 1.
- Board aspect matches Pi Zero; GPIO runs along the **long** edge.
- Display FPC is 24-pin 0.5 mm (`J2` / `PFC24/0.5MM` on schematic).
- Product “Outline dimensions” artwork: `https://www.waveshare.com/img/devkit/LCD/2.13inch-e-Paper-HAT/2.13inch-e-Paper-HAT-details-size.jpg` (linked from [product page](https://www.waveshare.com/product/raspberry-pi/2.13inch-e-paper-hat.htm)) — **fetch failed in this session** (HTTP 500/404 variants). **Gap:** exact pin-1 distance to board edges / FPC exit should be read from that drawing or measured on the physical HAT before cutting the perfboard keep-out.

---

## 7. Are the 8-pin cable and 40-pin header in parallel?

**Yes — same nets; either interface can drive the HAT.**

Evidence:

- Wiki: plug into 40PIN **or** use 8PIN cable with the correspondence table ([manual](https://www.waveshare.com/wiki/2.13inch_e-Paper_HAT_Manual)).
- Schematic: P3 (8-pin) VCC shares `RPI_5V` with header pins 2/4; GND shared; DIN/CLK/CS/DC/RST/BUSY are the same host-side nets that go to TXS0108E / panel ([schematic](https://files.waveshare.com/upload/5/57/2.13inch_e-Paper_HAT_Schematic.pdf)).

**Caveat:** do not drive the same net from both the shield header **and** the 8-pin cable at once.

---

## Primary sources (checklist)

| Source | URL |
| --- | --- |
| Wiki manual (pinout, dims, Rev2.1 notes) | https://www.waveshare.com/wiki/2.13inch_e-Paper_HAT_Manual |
| HAT schematic PDF | https://files.waveshare.com/upload/5/57/2.13inch_e-Paper_HAT_Schematic.pdf |
| Product page (65×30.2, features) | https://www.waveshare.com/product/raspberry-pi/2.13inch-e-paper-hat.htm |
| epdconfig.py | https://github.com/waveshareteam/e-Paper/blob/master/RaspberryPi_JetsonNano/python/lib/waveshare_epd/epdconfig.py |
| DEV_Config.c / .h | https://github.com/waveshareteam/e-Paper/blob/master/RaspberryPi_JetsonNano/c/lib/Config/ |
| C readme (Rev2.1 low power) | https://raw.githubusercontent.com/waveshareteam/e-Paper/master/RaspberryPi_JetsonNano/c/readme_EN.txt |
| Driver HAT wiki (PWR claim, Rev2.3) | https://www.waveshare.com/wiki/E-Paper_Driver_HAT — unverified this session |
| Field analysis of 40-pin 5 V path | http://martinhubacek.cz/blog/waveshare-epaper-hat-issues/ |

---

## Shield wiring cheat-sheet (nRF52, 3.3 V)

1. Male 2×20: 3.3 V into phys 2 and/or 4 (same net as 8-pin VCC; 3.3 V verified on the 8-pin cable). Do not feed 1/17.
2. GND → at least one of 6/9/14/20/25/30/34/39 (prefer several).
3. Signals: 19 DIN, 23 CLK, 24 CS, 22 DC, 11 RST, 18 BUSY.
4. Leave **1, 12, 17, 27, 28** and all other unused pins floating (or NC on the male header).
5. Keep RST high for power; avoid long RST-low pulses (Rev2.1 power gate).
6. Optional: ignore demo `PWR_PIN` for this HAT.
