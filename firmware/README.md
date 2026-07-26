# Firmware

Bare-metal C/C++ for the nRF52832 alarm clock (no Zephyr/RTOS application builds).

| Path | Purpose |
| --- | --- |
| `src/` | Application and drivers |
| `boards/` | Board/target notes (DK first, then custom PCB) |
| `docs/` | Bring-up notes, memory map, power states |

Stack and tooling constraints: repo-root [`AGENTS.md`](../AGENTS.md). Prototyping hardware: [`hardware/bom/proto-bom.md`](../hardware/bom/proto-bom.md).

SDK / build-system choice for the first in-tree app is intentionally not fixed here yet.
