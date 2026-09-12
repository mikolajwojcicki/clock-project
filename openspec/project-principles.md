# Clock Project Principles

This document preserves the repository's project-level principles independently
of any change-management framework. `AGENTS.md` remains the detailed operational
reference and must stay consistent with these principles.

## Core Principles

### 1. NixOS-native development

This workspace runs on NixOS. Do not suggest imperative package managers or
curl-based installers for system software. Durable system changes belong in
`~/thinkpad-nixos` and are activated with
`nixos-rebuild switch --flake ~/thinkpad-nixos#thinkpad`. Project-local tooling
must use Nix shells, flakes, or already-installed tools.

### 2. Bare-metal firmware

nRF52832 firmware must use bare-metal C/C++ with nRF5 SDK 17.1.0,
`arm-none-eabi-gcc`, and OpenOCD. Do not introduce Zephyr application builds,
FreeRTOS, `nrf-bm`, or another RTOS unless the user explicitly changes this
principle.

### 3. Locked hardware and product architecture

The architecture decisions in `AGENTS.md` are locked: Raytac
`MDBT42Q-512KV2`, Waveshare 2.13-inch e-paper, ST `LIS3DH`, `MCP73831`,
`AP2112K-3.3`, `AP-1205V-P1`, DCF-77 synchronization, and the nRF52 internal
RTC. Superseded STM32L0, SSD1306, external DS3231, and fully offline/no-BLE
choices must not be reintroduced without explicit user rollback.

### 4. Frequent, descriptive commits

Commit coherent work often, with imperative messages and area prefixes such as
`thesis:`, `hardware:`, `firmware:`, `web:`, and `docs:`. Keep unrelated thesis,
hardware, firmware, and documentation changes in separate commits. Do not
force-push, amend unrelated commits, alter Git configuration, or commit secrets
and generated build outputs.

### 5. Thesis writing fidelity

Thesis prose must follow `clock-project-latex/thesis-writing-rules.yaml`.
Typography must follow the WIT rules encoded in
`clock-project-latex/settings.tex`; explicit supervisor instructions override
both sources.

### 6. Clarify genuine user decisions

When an ambiguous requirement, architecture choice, or other genuine
user-owned decision blocks progress, ask the user with a structured question
instead of guessing. Safe, reversible defaults do not require clarification.

## Scope Separation

Keep the monorepo areas separate: `clock-project-latex/` for thesis LaTeX,
`hardware/` for KiCad and BOMs, `firmware/` for embedded code, `web/` for the
Web Bluetooth scaffold, and `temp-resources/` for university references. Place
new BOMs and pin maps under `hardware/`.

## Tooling

Prefer installed NixOS tooling (`gcc-arm-embedded`, `nrf5-sdk`, `openocd`,
`gdb`, and nRF Connect) before proposing new packages or downloads. System-wide
dependencies belong in `~/thinkpad-nixos`, not hand-edited generated files.

## Governance

`AGENTS.md` is the detailed operational companion. Where this document and
`AGENTS.md` disagree, update both as part of the same change and preserve the
more specific locked architecture decision. OpenSpec proposal, design, task,
and verification artifacts record workflow changes and their rationale.
