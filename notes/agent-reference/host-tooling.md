# Host nRF / embedded tooling (on-demand)

Read this when flashing, building firmware, or choosing SDK paths. Do not load unless needed.

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

