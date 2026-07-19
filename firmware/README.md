# Firmware

Bare-metal C/C++ for the nRF52832 alarm clock (**no Zephyr / RTOS** application builds).

| Path | Purpose |
| --- | --- |
| `apps/blinky/` | First bring-up: SoftDevice-free LED blinky on nRF52 DK (`PCA10040`) |
| `src/` | Shared application / driver code (later) |
| `boards/` | Board/target notes (DK first, then custom PCB) |
| `docs/` | Bring-up notes, memory map, power states |

## Stack (locked for nRF52832)

| Choice | Detail |
| --- | --- |
| SDK | **nRF5 SDK 17.1.0** via `NRF5_SDK_DIR` |
| Toolchain | `arm-none-eabi-gcc` via `GNU_INSTALL_ROOT` |
| Flash / debug | **OpenOCD** (on-board J-Link on the DK) |
| Not used here | Zephyr `west build`; **nRF Connect SDK Bare Metal (`nrf-bm`)** — nRF54L only |

Product constraints: repo-root [`AGENTS.md`](../AGENTS.md). Proto hardware: [`hardware/bom/proto-bom.md`](../hardware/bom/proto-bom.md).

## Quick start (blinky)

```bash
cd firmware/apps/blinky
make        # build
make flash  # OpenOCD → DK
```

VS Code: open the monorepo root, then **Run Task** → `firmware: blinky build` / `firmware: blinky flash`.
