# AGENTS.md

Bachelor's thesis monorepo (*praca inżynierska*) for an ultra-low-power alarm clock: custom PCB (KiCad), firmware, and LaTeX thesis. This workspace runs on **NixOS** — treat that as a hard constraint.

Canonical product/architecture decisions live **here**. `previous-compressed-conversation.md` is an archived handoff from an earlier LLM session; prefer this file if anything conflicts. Do not resurrect superseded choices (STM32L0, SSD1306 OLED, external DS3231, fully offline/no-BLE) unless the user explicitly rolls back.

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

Root holds shared docs (`AGENTS.md`, README), Nix flakes/shells, and CI. Keep concerns separated; split unrelated edits across commits. Commit prefixes: `thesis:` changes belong in the `clock-project-latex` repo (bump the submodule pointer in the parent when needed).

### Thesis LaTeX (finalized)

- **Working template:** official WIT `szablonwitpd` seeded into `clock-project-latex/` (`pdflatex` + `biber` / `biblatex`, UTF-8). Set `\stCzyMgr` to `0` for *inżynierska*.
- **Writing rules (mandatory for agents):** before drafting or editing thesis prose, read and follow [`clock-project-latex/thesis-writing-rules.yaml`](clock-project-latex/thesis-writing-rules.yaml) (machine-readable extract of Greber *Zasady pisania prac dyplomowych*, wyd. VI). Use it for structure, wstęp/zakończenie, figures/tables, citations, and typography pitfalls. Chapter-2 “organizacja” framing is adapted for this hardware/firmware thesis (see the YAML). Promotor overrides win on conflict. Source PDF remains in `temp-resources/Zasady-pisania-prac-dyplomowych.pdf`.
- **Typography:** WIT deanery rules (`temp-resources/wymogi_edytorskie.docx.md`) are already in `clock-project-latex/settings.tex` (Times/`newtx`, sizes 14/13/12, single spacing). Prefer those over Greber’s 1.5 spacing / 35 mm left margin when they disagree — the YAML documents both.
- **CI PDF:** every push/PR in `clock-project-latex` runs `.github/workflows/build-thesis.yml` and uploads artifact `thesis-pdf` (`main.pdf`). Prefer this over local TeX when agents need the built PDF. Retrieve with:
  `gh run download -R mikolajwojcicki/clock-project-latex -n thesis-pdf` (optionally `-R` / `--dir` / run id from `gh run list`).
- **Documentation only:** Kubik memoir v0.9 kept under `temp-resources/docs/memoir-v09/` (+ original `temp-resources/dyplomszablonmemoir-v09.zip`). CP1250 / Overleaf-hostile; do not compile as the thesis tree. Steal typography ideas into WIT `settings.tex` if needed.
- **Removed:** English-first `szablon_pracy_dyplomowej_w_latex` (deleted from `temp-resources/`).
- Also in `temp-resources/`: approved APD topic PDF `my-thesis.pdf`, `Zasady-pisania-prac-dyplomowych.pdf`, `wymogi_edytorskie.docx.md`, `pd_inz_pl.doc.pdf`, `skroty_kier_pd.xls`, upstream `szablonwitpd.zip`.

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
| Display | Waveshare 2.13" e-Paper module | **Option 2:** 1×8 2.54 mm header to breakout (HV boost on module), not raw FPC + discrete boost |
| Time sync | DCF-77 receiver module (e.g. MAS6180-class) | Use PD/power-down; enable only for periodic sync |
| RTC | nRF52 internal RTC + 32.768 kHz crystal | **No** external DS3231 |
| Motion | ST `LIS3DH` (`LIS3DHTR`) | INT1/INT2 wake; no polling while sleeping |
| Charger | Microchip `MCP73831` | USB-C input |
| LDO | Diodes `AP2112K-3.3` | 3.3 V rail |
| Buzzer | `AP-1205V-P1` electromagnetic | **Option A** (loud); NPN driver (`MMBT3904` / equiv.) + flyback diode; accept ~50 mA while sounding |
| Debug | SWD header | 1×4 |
| Firmware stack | **Bare-metal** on nRF52832 | **No Zephyr**, no RTOS — see below |

### Firmware stack (locked)

- **Bare-metal only** (C/C++ on the Nordic nRF52832 / SoftDevice or SoftDevice-free as decided later).
- **Do not** introduce Zephyr RTOS application builds, FreeRTOS, or another RTOS unless the user explicitly changes this decision.
- Rationale: the thesis prioritizes measurable ultra-low-power behavior (sleep currents, peripheral power-gating, interrupt-driven wakes). A thin bare-metal control loop gives direct ownership of clock trees, POWER/CLOCK peripherals, and sleep entry/exit without RTOS scheduling or subsystem abstractions obscuring the energy story.
- **For nRF52832 / nRF52 DK / MDBT42Q:** use **nRF5 SDK 17.1.0** (`NRF5_SDK_DIR`) + `arm-none-eabi-gcc` + **OpenOCD** flash. First in-tree app: [`firmware/apps/blinky/`](firmware/apps/blinky/) (SoftDevice-free / blank).
- **Do not** use VS Code nRF Connect → Create application → Copy sample from **`nrf-bm`** for this hardware. Nordic’s **nRF Connect SDK Bare Metal** (`~/ncs/nrf-bm`) targets **nRF54L only** — incompatible with PCA10040 / nRF52832.
- Prefer Nordic NRFx / CMSIS / nRF5 SDK examples — not Zephyr `west build` app samples.
- BLE (later): Nordic SoftDevice via nRF5 SDK; keep application power policy in project-owned code.
- When suggesting tooling, prefer what is **already installed** on this machine (see below) before proposing new Nix packages or downloads.

### Installed nRF / embedded tooling (this NixOS host)

Do **not** tell the user to `apt install` toolchains or re-download SDKs that are already present. Prefer these paths and packages.

#### System packages (via `~/thinkpad-nixos`, on PATH / session env)

| Tool | Status | Notes |
| --- | --- | --- |
| `gcc-arm-embedded` (`arm-none-eabi-gcc` …) | Installed | Arm GNU Toolchain **15.2.Rel1**; `GNU_INSTALL_ROOT` points at its `bin/` |
| `nrf5-sdk` | Installed | Nordic **nRF5 SDK 17.1.0**; `NRF5_SDK_DIR` → store `…/share/nRF5_SDK` |
| `segger-jlink-headless` | Installed | J-Link libs + udev (`99-jlink.rules`); license accepted in nixpkgs config |
| `nrf-udev` | Installed | USB perms for Nordic DKs / probes |
| `openocd` | Installed | **0.12.0** — primary flash/debug path noted in system config (alongside J-Link) |
| `gdb` | Installed | Host debugger |
| `nrfconnect` | Installed | nRF Connect for Desktop (GUI) |
| `nrfconnect-bluetooth-low-energy` | Installed | BLE app for nRF Connect for Desktop |
| `gnumake`, `cmake`, `python3`, `picocom` | Installed | Build + UART console |
| `nrfutil` (nixpkgs) | **Not** packaged system-wide | Omitted in configuration due to a nixpkgs packaging quirk; flash via OpenOCD / nRF Connect / extension tooling instead |
| `nrfjprog` / `JLinkExe` on PATH | Not exposed as plain CLI wrappers | Use OpenOCD, nRF Connect for Desktop, or tools bundled with the VS Code / `~/ncs` toolchain |

Session variables (already set system-wide):

- `NRF5_SDK_DIR` — nRF5 SDK 17.1.0 root (Makefile-friendly)
- `GNU_INSTALL_ROOT` — trailing-slash path to `arm-none-eabi-*` binaries (nRF5 SDK Makefile convention)

VS Code is installed as **`vscode.fhsWithPackages`**, with `gcc-arm-embedded`, `gdb`, `openocd`, `python3`, `gnumake`, `cmake`, and `picocom` visible inside the FHS bubble (needed for Marketplace extension native binaries / Cortex-Debug-style workflows).

#### User VS Code + nRF Connect (Marketplace)

The user develops in **VS Code** with the Nordic **nRF Connect** extension ecosystem installed under `~/.vscode/extensions/`:

- `nordic-semiconductor.nrf-connect` (2026.7.x)
- `nordic-semiconductor.nrf-connect-extension-pack`
- `nordic-semiconductor.nrf-terminal`
- `nordic-semiconductor.nrf-devicetree`
- `nordic-semiconductor.nrf-kconfig`

Associated SDK / toolchain installs managed via nRF Connect (under `~/ncs/`):

| Path | What it is |
| --- | --- |
| `~/ncs/nrf-bm/v2.0.0` | **nRF Connect SDK Bare Metal** (**nrf-bm 2.0.0**) — **nRF54L only**; installed but **not** used for this nRF52832 thesis |
| `~/ncs/toolchains/911f4c5c26` | nRF Connect toolchain bundle (nrfutil-managed; linked for NCS **v3.3.0** in `toolchains.json`) |
| `~/.nrfconnect-apps/` | nRF Connect for Desktop app manifests (Programmer, Toolchain Manager, BLE, PPK, etc.) |

Note: `nrf-bm` may ship sibling `zephyr/` trees for Nordic packaging; ignore those for application builds. For this monorepo, stay on **nRF5 SDK** under `firmware/apps/` (see blinky). The nRF Connect extension remains useful for **Connected Devices** / Programmer UI; build and flash the in-tree apps with Make + OpenOCD (`.vscode/tasks.json`).

### nRF52 / module layout constraints (do not ignore)

- Pins `P0.22`–`P0.30` (module pins near radio): low-drive, low-frequency only (&lt;10 kHz) — buttons OK; **not** SPI/PWM
- Map SPI (e-paper) to safer GPIOs in `P0.11`–`P0.20` range (exact pinout TBD in schematic)
- Antenna keep-out ~3.8 mm on all layers under chip antenna; module at PCB edge; via stitching
- Enable internal DC-DC: external `L2` 10 µH, `L3` 15 nH, `C14` 1 µF per Raytac/nRF guidance
- External 32.768 kHz crystal + 12 pF load caps

### Prototyping path (before / beside custom PCB)

Use breadboard bring-up with: nRF52 DK (`PCA10040`), Waveshare 2.13" e-Paper, Adafruit LIS3DH breakout, DCF-77 module, THT buzzer + `2N3904`/`BC547` + `1N4148`, optional TP4056 + Li-Po for power experiments. **Proto kit is on hand** — checklist in [`hardware/bom/proto-bom.md`](hardware/bom/proto-bom.md). **Production PCB BOM:** [`hardware/bom/pcb-bom.md`](hardware/bom/pcb-bom.md). A Zephyr blink on the DK was exploratory only; the in-tree bare-metal blinky is [`firmware/apps/blinky/`](firmware/apps/blinky/) (nRF5 SDK).

### Thesis scope (expected chapters / work)

- Theory: embedded low-power methods, interrupt vs polling, DCF-77 coding, BLE overview
- Hardware: schematic, component selection, energy budget, PCB with EMC care (RF + DCF analog)
- Firmware: C/C++, DCF decode, e-paper driver, sleep/wake, alarm + motion dismiss
- Web: Web Bluetooth schedule upload
- Research: measure current in sleep / BLE / DCF / alarm vs theoretical budget

## Git workflow (mandatory for agents)

Commit **often** and with **descriptive** messages. Prefer many small, reviewable commits over one large dump at the end of a session.

**Thesis rationale (promotor guidance):** a dense, well-described commit history makes the *praca inżynierska* easier to write later — you can reconstruct what was tried, when architecture changed, and what to put in the firmware/hardware chronology chapters. Sparse or squash-style history is a liability for the paper. Bias hard toward **more commits**, not fewer.

### When to commit

- After a coherent unit of work is complete and working (or intentionally checkpointed)
- After finishing one concern: e.g. a LaTeX section, a PCB net fix, a firmware driver stub — not all three in one commit if they are unrelated
- After a successful build/flash milestone (e.g. blinky on DK) — commit the app *before* unrelated docs if both are dirty
- Before switching tasks or ending a substantial agent turn that changed tracked files
- When the user asks to commit (always honor that immediately)
- Prefer splitting a session’s work into **several** commits (app → build/flash → editor tasks → AGENTS/docs) rather than one catch-all

Do **not** wait until the entire feature/thesis chapter is "done" before the first commit. Do **not** batch a whole evening of unrelated progress into a single commit to “keep the log clean.”

### How to commit

- In this repo, agents **should create commits** as work lands — that is intentional for the thesis history and for later chapter drafting from `git log`. Do not wait to be asked for each small unit once work is underway in a session; still honor an explicit “don’t commit yet” from the user.
- Never update git config. Never force-push to shared branches. Never amend unless the user explicitly requests it and amend safety conditions hold (commit is yours, not pushed, etc.).
- Never update git config. Never force-push to shared branches. Never amend unless the user explicitly requests it and amend safety conditions hold (commit is yours, not pushed, etc.).
- Do not commit secrets, private keys, `.env`, KiCad autosave/backup junk, LaTeX build outputs, or large generated binaries unless the user explicitly wants those artifacts tracked.
- Stage only relevant files for the commit (avoid `git add .` when unrelated changes exist).
- Pass the message via a HEREDOC. Subject focuses on **why**, not a file list. Prefer imperative mood.

### Message style

```text
Good:
  thesis: outline low-power interrupt architecture section
  hardware: reserve antenna keep-out for MDBT42Q module
  firmware: wake on LIS3DH INT1 for alarm dismiss

Bad:
  Update files
  WIP
  changes
  Fix stuff
```

- Subject ≈ 50–72 characters when practical; add a body when the why is non-obvious
- Prefer prefixes: `thesis:`, `hardware:`, `firmware:`, `web:`
- One logical change per commit; split mixed LaTeX + PCB + firmware edits when independent

### Suggested cadence

| Kind of work | Commit when… |
| --- | --- |
| LaTeX | A section/subsection, figure set, or bib change is complete |
| KiCad | A schematic/PCB decision is saved and consistent (e.g. net rename + matching PCB update) |
| Firmware | A module builds (or a failing test is an intentional checkpoint) and behavior is describable — then a separate commit for flash/docs/tasks if those follow |
| Docs / AGENTS.md | Decision locks and workflow rules change — own commit, not folded into unrelated code |

If unsure whether to commit: **commit**. A slightly granular history beats a missing one when writing the thesis.

## Host facts (NixOS)

- OS: NixOS (declarative, immutable system model)
- System config flake: `~/thinkpad-nixos` (`nixosConfigurations.thinkpad`)
- Apply system changes with: `sudo nixos-rebuild switch --flake ~/thinkpad-nixos#thinkpad`
- Do **not** assume a traditional FHS layout or a mutable package database

## What NixOS is (for agents)

NixOS is not “Linux with a different package manager.” The running system is a build product of Nix expressions. Packages live in the Nix store (`/nix/store/...`) and are wired into profiles via symlinks. There is no `apt`, `dnf`, `pacman`, or `brew`-style install that permanently mutates the OS.

| Traditional distro habit | NixOS reality |
| --- | --- |
| `apt install foo` / `dnf install foo` | Add to `environment.systemPackages` (or Home Manager / a flake), then rebuild |
| Edit files under `/usr` or `/etc` permanently | Prefer declarative config in the flake; many `/etc` files are generated and overwritten on rebuild |
| “Put it in `/usr/local/bin`” | Put it in a Nix derivation, `environment.systemPackages`, or a user profile |
| Assume `/bin/bash`, `/usr/bin/env`, and FHS paths everywhere | Paths often resolve through `/run/current-system/sw` or nix-shell/devShell; use `#!/usr/bin/env` carefully and prefer Nix-provided interpreters |
| Install build deps globally to “just compile” | Use `nix-shell`, `nix develop`, or a project `flake.nix` / `shell.nix` |
| Enable a service by writing a unit and `systemctl enable` | Prefer `services.*.enable = true;` (or a custom systemd module) in NixOS config, then rebuild |

## Hard rules for this machine

1. **Never suggest imperative package managers** (`apt`, `yum`, `dnf`, `pacman`, `snap`, flatpak-as-default-install path) for system software.
2. **Never tell the user to hand-edit generated system files** as the durable fix. If a change must survive reboot/rebuild, it belongs in `~/thinkpad-nixos` (or a project flake).
3. **Prefer flakes and nix-shell/devShells** for project tooling. Do not pollute the global system with one-off compile dependencies when a shell can provide them.
4. **Rebuild is the activation step.** After changing NixOS modules/packages, the change is not live until `nixos-rebuild switch` (or equivalent) succeeds.
5. **Check what Nix already provides** before inventing wrappers, vendoring binaries, or downloading install scripts meant for other distros.
6. **Secrets and Wi-Fi credentials** under `~/thinkpad-nixos/secrets/` are sensitive — do not commit, print, or paste them into chat logs casually.

## Working in this monorepo

- Keep the project **Nix-friendly**: prefer the already-installed system `gcc-arm-embedded` + `nrf5-sdk` (`NRF5_SDK_DIR` / `GNU_INSTALL_ROOT`) and OpenOCD — not Zephyr app workflows, not `nrf-bm` for nRF52832, and not fresh distro-wide reinstalls.
- If a dependency must be system-wide (udev for J-Link/nRF DK, etc.), propose the change in `~/thinkpad-nixos`.
- Prefer NixOS-native commands (`nix`, `nix-shell`, `nix develop`, `nixos-rebuild`) over distro install guides.
- Do not mix unrelated thesis/hardware/firmware edits in one commit when they can be split cleanly.
- When adding BOMs or pin maps, put them under `hardware/` (and reference from the thesis) rather than only in chat.

## Quick decision guide

- **Need a CLI tool once for development?** → `nix-shell -p <pkg>` or a project devShell.
- **Need a tool always on PATH for the user/system?** → add to Home Manager / `environment.systemPackages`, rebuild.
- **Need a background service / hardware integration?** → NixOS module in `~/thinkpad-nixos`, rebuild.
- **Need to debug “command not found”?** → check `which`, `type`, and whether you are inside a nix-shell/devShell; do not assume Ubuntu package names map 1:1.

## Anti-patterns to avoid

- Curl-bashing install scripts that assume glibc FHS layouts or write into `/usr`.
- Instructing `chmod +x` fixes for missing dynamic loaders without considering Nix packaging.
- Assuming toolchains from the OS image are complete; pin them in the project flake/shell instead.
- Treating `/etc/nixos` as the only config location — durable system config is `~/thinkpad-nixos`.
- Giant catch-all commits that bury PCB, firmware, and LaTeX changes together without need.
- Reintroducing deleted architecture (OLED-only offline STM32 clock) without an explicit user decision.
- Pulling in Zephyr/west or an RTOS “for convenience” — conflicts with the bare-metal power-efficiency thesis focus.
