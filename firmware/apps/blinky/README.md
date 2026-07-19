# Blinky (nRF52 DK)

SoftDevice-free LED blinky for the **nRF52 DK** (`PCA10040` / nRF52832), built with **nRF5 SDK 17.1.0** and `arm-none-eabi-gcc`.

This is intentionally **not** an nRF Connect SDK Bare Metal (`nrf-bm`) or Zephyr app — those paths do not match this board / thesis stack. See repo-root [`AGENTS.md`](../../../AGENTS.md).

## Prerequisites

Already set on this NixOS host:

- `NRF5_SDK_DIR` — nRF5 SDK 17.1.0 root
- `GNU_INSTALL_ROOT` — Arm GNU toolchain `bin/` (trailing slash)
- `openocd` on `PATH` (on-board J-Link)

Plug in the DK over USB.

## Build

From this directory:

```bash
make
```

Artifacts land in `armgcc/_build/` (`nrf52832_xxaa.hex`, `.out`, `.bin`).

## Flash

```bash
make flash
```

Uses OpenOCD (`interface/jlink.cfg` + `target/nrf52.cfg`). Mass-erase:

```bash
make erase
```

## VS Code

Open the monorepo root (`clock-project`). Then **Terminal → Run Task…** → **firmware: blinky build** or **firmware: blinky flash** (tasks in [`.vscode/tasks.json`](../../../.vscode/tasks.json)).

Do **not** use nRF Connect → Create a new application → Copy a sample from `nrf-bm` for this kit.
