# nRF52 DK isolation + battery power (on-demand)

Read for sleep-current / battery-prototype work. Prefer linked scratch docs for full evidence.

#### nRF52 DK solder-bridge isolation (PCA10040) — required for battery/no-USB prototype

The final thesis prototype is **not USB-powered** (battery / PPK2 on the nRF rail). On the DK, the interface MCU (IMCU) ↔ nRF **UART** path leaks **both ways** through ESD/GPIO coupling: USB-powered IMCU can hold `VDD_nRF` up with PPK2 off (~1.5–2.7 V observed; TP6/TP8 live), and with USB unplugged an unpowered IMCU loads a live nRF rail. For trustworthy sleep-current work and a battery-only bring-up, cut the isolation bridges — do not rely on “no terminal attached.”

| Set | Bridges | Status / action | Why |
| --- | --- | --- | --- |
| Supply series | **SB9** | Already cut | Puts P22 in series for PPK2 |
| SWD / RESET / SWO | **SB20, SB21, SB26, SB27** | Already cut | Isolates on-board debugger SWDIO/SWDCLK/RESET/SWO |
| UART / VCOM | **SB22, SB23, SB24, SB25** | **Cut these** (bench confirmed residual on **SB23**/TP6 and **SB25**/TP8; cut all four) | Bidirectional IMCU↔nRF leak on `P0.05`–`P0.08`; critical when IMCU has no USB power |
| Do **not** cut | **SB29** | Leave closed | Cutting IMCU power makes back-feed worse while USB is used for flash/debug |

Notes for agents:
- Flash after UART cuts via temporary **P19→P18** ribbon; **unplug the ribbon** before every current measurement (pin 1 VTG re-ties domains).
- **P19→P18 does not carry UART.** It is SWD only (VTG, SWDIO, SELECT, SWDCLK, GND, SWO, RESET — UG Table 6). USB VCOM is a separate IMCU path through SB22–SB25.
- There is **no dedicated UART header** on the DK. After SB22–SB25 are cut, USB CDC console to the nRF is gone.
- **Console workaround (on hand):** use the user's **external USB–UART adapter** on the Arduino / GPIO headers (**P13–P17** / Arduino footprints). `P0.05`–`P0.08` remain available there through **SB13–SB16** (connector path — distinct from IMCU VCOM). Leave SB13–SB16 closed. Typical wiring: adapter RX←`P0.06` (TXD), TX→`P0.08` (RXD), GND common; RTS/CTS optional on `P0.05`/`P0.07`.
- **TP5–TP8** are VCOM test points on the **IMCU side** of SB22–SB25 — useful to confirm the leak; not a post-cut console to the nRF.
- SW7 OFF only drops RTS/CTS (`P0.05`/`P0.07`); it does **not** fix TX/RX leak (`SB23`/`SB25`).
- Detail / evidence: [`scratch/sb-isolation-research/`](scratch/sb-isolation-research/) (`PLAN.md`, `QUESTIONS.md`, `catalog.json`).

#### nRF52 DK battery power via `P1` `5V` (locked prototype path)

When the DK USB cable is **unplugged**, the pin silk-screened **`5V` on header `P1`** may be used as a **battery input**. A single-cell Li-Po (~3.0–4.2 V) into that pin feeds the DK onboard `AP7333` LDO and produces the board `3.3 V` rail. This is a known Nordic DK workaround (also used in Nordic’s hexapod blog); it is **not** one of the three official UG power options (USB / `P21` 1.7–3.6 V / coin cell).

| Do | Do not |
| --- | --- |
| Battery → slide switch → **`P1` `5V`** + buzzer `+`; GND → **`P1` `GND`** | Put Li-Po on **`P21`** or on **`P1` `VDD`/`3V3`** (max 3.6 V, unregulated / logic rail) |
| Keep **DK USB unplugged** while BAT feeds `P1` `5V` | Dual-feed USB 5 V and battery onto the same `5V` net |
| Charge on **Amigo USB-C** with the load switch **OFF** | Treat Amigo’s silk `5V` pad as a battery-boosted 5 V output (it is USB VBUS input only) |
| Prefer Pimoroni LiPo Amigo / Amigo Pro (`MCP73831`, JST-PH) + ~500 mAh JST-PH cell | Leave Fast Charge bridged on Amigo with a ~500 mAh cell |

Operating modes:
1. **Flash / USB bring-up:** DK USB on; battery switch **OFF** (no BAT on `P1` `5V`).
2. **Battery demo / sleep current:** DK USB out; switch **ON**; keep SB UART isolation rules above.
3. **Charge:** USB-C into Amigo only; switch preferably **OFF**.

Full wiring / buzzer / escape-hatch notes: [`scratch/battery-and-charger/power-architecture.md`](scratch/battery-and-charger/power-architecture.md). Parts pick: [`scratch/battery-and-charger/recommendation.md`](scratch/battery-and-charger/recommendation.md).

