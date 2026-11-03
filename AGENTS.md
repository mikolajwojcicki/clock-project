# AGENTS.md

Bachelor's thesis monorepo (*praca inżynierska*) for an ultra-low-power alarm clock: custom PCB (KiCad), firmware, and LaTeX thesis. This workspace runs on **NixOS** — treat that as a hard constraint.

Canonical product/architecture decisions live **here**. `previous-compressed-conversation.md` is an archived handoff from an earlier LLM session; prefer this file if anything conflicts. Do not resurrect superseded choices (STM32L0, SSD1306 OLED, external DS3231, fully offline/no-BLE) unless the user explicitly rolls back.

**Token hygiene:** keep this file short. Deep host/tooling, DK isolation/power, and NixOS primers live under [`notes/agent-reference/`](notes/agent-reference/) — read those **only when the task needs them**. Prefer fresh sessions after long OpenSpec/apply loops; watch prompt-cache thrash (stuck ~18k `cache_read` + huge `cache_write`).

## Project purpose

Official APD thesis topic (approved):

> Projekt i realizacja energooszczędnego budzika z bezprzewodową konfiguracją oraz radiową synchronizacją czasu DCF-77

English title: *Design and implementation of a low-power alarm clock with wireless configuration and DCF-77 radio time synchronization*

Goal: battery-powered bedside alarm clock that avoids keeping a phone in the bedroom, with per-weekday alarm schedules, while optimizing for **ultra-low power**. Dismissing the alarm requires physical activity (shake / jump-jack style motion), detected by accelerometer interrupts — in the APD description/scope, not in the registered title.

### Monorepo layout

| Area | Contents | Path |
| --- | --- | --- |
| Thesis paper | LaTeX sources (separate git repo / submodule) | `clock-project-latex/` |
| Hardware | KiCad schematic/PCB, libs, fab exports, BOMs | `hardware/` |
| Firmware | Embedded C/C++, build/flash, drivers | `firmware/` |
| Companion UI | Web Bluetooth alarm-schedule app | `web/` (scaffold only for now) |
| University reference materials | Editorial rules, title form, codes, memoir docs | `temp-resources/` |
| Agent on-demand refs | Host tooling, DK power, NixOS primer, OMP tuning | `notes/agent-reference/` |

Root holds shared docs (`AGENTS.md`), Nix flakes/shells, and CI. Keep concerns separated; split unrelated edits across commits. Commit prefixes: `thesis:` changes belong in the `clock-project-latex` repo (bump the submodule pointer in the parent when needed). Nested `/scratch/` is its own git repo (ignored here).

### Thesis LaTeX (finalized)

- **Working template:** official WIT `szablonwitpd` in `clock-project-latex/` (`pdflatex` + `biber` / `biblatex`, UTF-8). `\stCzyMgr` = `0` for *inżynierska*.
- **Writing rules:** before drafting/editing thesis prose, read [`clock-project-latex/thesis-writing-rules.yaml`](clock-project-latex/thesis-writing-rules.yaml). Chapter order (promotor 2026-10-13): 3→4→5→2→1→6. Promotor overrides win on conflict.
- **Draft, then rewrite:** agents may draft; author must rewrite kept prose and log it in [`notes/ai-use.md`](notes/ai-use.md). Agents must **not** draft "Informacja o wykorzystaniu narzędzi AI".
- **Evidence log:** dated notes under [`notes/`](notes/). Screenshots: light theme, `YYYY-MM-DD_topic_detail.ext`. PPK2: pack via `firmware/tools/ppk-plot/ppk-archive.sh`; tracked `.ppkz` &lt; 50 MB; CSV stays local (`notes/measurements/ppk-*.csv` ignored).
- **People as test subjects:** wait for promotor ethics filing (self-test / instruments OK).
- **CI PDF:** `gh run download -R mikolajwojcicki/clock-project-latex -n thesis-pdf`.
- Detail paths: `temp-resources/` (APD PDF, Greber rules, deanery typography, memoir archive). Prefer WIT `settings.tex` over Greber spacing/margins when they disagree.

## Product requirements (current)

### Functional

- Timekeeping: hours, minutes, seconds, day of week; user-settable; corrected by periodic DCF-77 sync
- Alarms: unique HH:MM per weekday; enable/disable per day; audible alarm; silence via required motion activity (accelerometer), plus minimal buttons
- UI: e-paper display (time, day, alarm status, low battery); few physical buttons
- Config path: BLE + Web Bluetooth web app to upload schedules (not Wi-Fi)
- Power: rechargeable Li-Po/Li-Ion, USB-C charging, charge-status LED

### Non-functional

- Ultra-low power: deep sleep on MCU; e-paper (power only on refresh); interrupt-driven wakes (buttons, accelerometer INT, RTC ticks, timed DCF windows); power-gate DCF receiver when idle
- Compact device; no Wi-Fi; BLE only for configuration
- Reliable time with low drift (32.768 kHz crystal + daily DCF-77 calibration)
- Cost-conscious BOM; Poland-friendly sourcing (TME / Botland / Allegro where noted)

## Architecture decisions (locked unless user changes them)

| Block | Choice | Notes |
| --- | --- | --- |
| MCU | Raytac `MDBT42Q-512KV2` (nRF52832 module) | Pre-certified; TME `MDBT42Q-512KV2`; ship-to-Poland OK |
| Display | Waveshare 2.13" e-Paper HAT (Rev2.1) | **Option 2:** module with HV boost on board, not raw FPC + discrete boost. Perfboard: HAT plugs onto a 2×20 RPi male header (decision 2026-11-03; 3V3 fed into RPi pin 2/4, see `hardware/docs/waveshare-hat-40pin.md`) |
| Time sync | DCF-77 receiver module (e.g. MAS6180-class) | Use PD/power-down; enable only for periodic sync |
| RTC | nRF52 internal RTC + 32.768 kHz crystal | **No** external DS3231 |
| Motion | ST `LIS3DH` (`LIS3DHTR`) | INT1/INT2 wake; no polling while sleeping |
| Charger | Microchip `MCP73831` | USB-C input |
| LDO | Diodes `AP2112K-3.3` | 3.3 V rail |
| Buzzer | `AP-1205V-P1` electromagnetic | **Option A** (loud); NPN driver + flyback; ~14 mA on `5V` while sounding (see deep-sleep notes) |
| Debug | SWD header | 1×4 |
| Firmware stack | **Bare-metal** on nRF52832 | **No Zephyr**, no RTOS — see below |

### Firmware stack (locked)

- **Bare-metal only** (C/C++ on nRF52832; SoftDevice or SoftDevice-free as decided per feature).
- **Do not** introduce Zephyr/FreeRTOS/other RTOS unless the user explicitly changes this.
- **For nRF52832 / DK / MDBT42Q:** **nRF5 SDK 17.1.0** (`NRF5_SDK_DIR`) + `arm-none-eabi-gcc` + **OpenOCD**. First in-tree app: [`firmware/apps/blinky/`](firmware/apps/blinky/).
- **Do not** use nRF Connect SDK Bare Metal (`nrf-bm`) — it is **nRF54L only**.
- BLE: Nordic SoftDevice via nRF5 SDK; keep power policy in project-owned code.
- Host paths / VS Code / `~/ncs` layout: [`notes/agent-reference/host-tooling.md`](notes/agent-reference/host-tooling.md) (on-demand).

### nRF52 / module layout constraints (do not ignore)

- Pins `P0.22`–`P0.30`: low-drive, low-frequency only (&lt;10 kHz) — buttons OK; **not** SPI/PWM
- Map SPI (e-paper) to safer GPIOs in `P0.11`–`P0.20` range (exact pinout TBD)
- Antenna keep-out ~3.8 mm; module at PCB edge; via stitching
- Enable internal DC-DC: external `L2` 10 µH, `L3` 15 nH, `C14` 1 µF
- External 32.768 kHz crystal + 12 pF load caps

### Prototyping path (short)

Breadboard/perfboard bring-up with nRF52 DK (`PCA10040`), Waveshare 2.13" e-Paper, LIS3DH, DCF-77 module, THT buzzer + NPN + diode, Li-Po + Pimoroni LiPo Amigo. Proto checklist: [`hardware/bom/proto-bom.md`](hardware/bom/proto-bom.md). Thesis prototype is **soldered perfboard** (no jumper wires). Custom PCB optional later: [`hardware/bom/pcb-bom.md`](hardware/bom/pcb-bom.md).

**Sleep-current / battery work:** cut DK UART isolation bridges and use `P1` `5V` battery feed per [`notes/agent-reference/dk-prototype-power.md`](notes/agent-reference/dk-prototype-power.md). Evidence also under `scratch/sb-isolation-research/` and `scratch/battery-and-charger/` (scratch is a nested repo).

### Thesis scope (expected chapters / work)

- Theory: low-power methods, interrupt vs polling, DCF-77, BLE overview
- Hardware: schematic, selection, energy budget, EMC care (RF + DCF analog)
- Firmware: DCF decode, e-paper, sleep/wake, alarm + motion dismiss
- Web: Web Bluetooth schedule upload
- Research: measure current in sleep / BLE / DCF / alarm vs budget

## Asking the user (mandatory for agents)

Every question to the user MUST go through the interactive `ask` tool, never as plain chat text. Grill rounds: one `ask` call per round, one entry per question, recommended option first.

## Git workflow (mandatory for agents)

Commit **often** with descriptive messages. Project rule [`.omp/rules/thesis-commit-history.mdc`](.omp/rules/thesis-commit-history.mdc) is authoritative (overrides “only commit when asked”). Prefixes: `thesis:`, `hardware:`, `firmware:`, `web:`. No force-push; no secrets; stage related files only; HEREDOC messages. If unsure: **commit**.

## Host facts (NixOS)

- OS: NixOS; system flake `~/thinkpad-nixos` (`nixosConfigurations.thinkpad`)
- Apply: `sudo nixos-rebuild switch --flake ~/thinkpad-nixos#thinkpad`
- Do **not** assume traditional FHS or `apt`/`dnf`. Primer: [`notes/agent-reference/nixos-for-agents.md`](notes/agent-reference/nixos-for-agents.md)

## Hard rules for this machine

1. Never suggest imperative package managers for system software.
2. Never treat hand-edits under `/etc` as the durable fix — use `~/thinkpad-nixos`.
3. Prefer flakes / `nix-shell` / `nix develop` for project tooling.
4. Rebuild activates system changes.
5. Check what Nix already provides before curl-bash installs.
6. Secrets under `~/thinkpad-nixos/secrets/` stay out of chat/commits.

## Working in this monorepo

- Prefer installed `gcc-arm-embedded` + `nRF5 SDK` + OpenOCD — not Zephyr/`nrf-bm` for this MCU.
- System-wide needs (udev, etc.): propose in `~/thinkpad-nixos`.
- Put BOMs/pin maps under `hardware/`; dated bring-up notes under `notes/`.
- Optional MCP recipes / OMP global tuning: [`notes/agent-reference/omp-token-hygiene.md`](notes/agent-reference/omp-token-hygiene.md).

## Anti-patterns to avoid

- Curl-bash FHS install scripts; Zephyr/RTOS “for convenience”; resurrecting deleted architecture without an explicit user decision.
- Giant catch-all commits; reading huge dumps (`notes/measurements/`, multi-MB datasheet `.txt`) into context when a search/slice will do.
- Leaving Overleaf/API tokens in tracked `.omp/mcp.json`.
