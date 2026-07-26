<!--
Sync Impact Report
==================
Version change: TEMPLATE (unratified) → 1.0.0
Bump rationale: Initial ratification — first concrete constitution for this repo.

Modified principles: n/a (initial ratification, no prior concrete principles)

Added sections:
  - Core Principles I–VI (NixOS-Native Development, Bare-Metal Firmware/No RTOS,
    Locked Hardware & Product Architecture, Frequent Descriptive Commit History,
    Thesis Writing Fidelity to Institutional Rules, Mandatory Clarification via
    AskQuestion)
  - Monorepo Structure & Scope Separation
  - Development Workflow & Tooling Standards
  - Governance

Removed sections: none (template placeholders replaced with concrete content)

Templates requiring updates:
  - .specify/templates/plan-template.md ✅ compatible — Constitution Check gate
    reads this file at plan time; no template edits needed.
  - .specify/templates/spec-template.md ✅ compatible — no constitution-specific
    references to reconcile.
  - .specify/templates/tasks-template.md ✅ compatible — no constitution-specific
    references to reconcile.
  - AGENTS.md ⚠ pending (manual, non-blocking) — AGENTS.md remains the detailed
    operational companion to this constitution (full architecture tables, tool
    inventories, commit-message examples). Per Governance below, AGENTS.md MUST
    stay consistent with these principles; no contradictions were found at
    ratification time, but future edits to either file should cross-check the
    other.

Follow-up TODOs: none blocking.
-->

# Projekt i realizacja energooszczędnego budzika z bezprzewodową konfiguracją oraz radiową synchronizacją czasu DCF-77 — Constitution

*Low-power alarm clock with wireless configuration and DCF-77 radio time synchronization (bachelor's thesis monorepo: LaTeX, KiCad hardware, embedded firmware).*

## Core Principles

### I. NixOS-Native Development (NON-NEGOTIABLE)

This workspace runs on NixOS, an immutable, declarative system — not "Linux with a
different package manager." Agents MUST NOT suggest imperative package managers
(`apt`, `yum`, `dnf`, `pacman`, `snap`) or curl-bash installers for system software.
Agents MUST NOT instruct hand-editing generated system files as a durable fix; any
change that must survive a reboot/rebuild belongs in the `~/thinkpad-nixos` flake,
activated via `nixos-rebuild switch --flake ~/thinkpad-nixos#thinkpad`. Project-local
tooling MUST use `nix-shell`, `nix develop`, or a project flake/shell instead of
global installs. Rationale: traditional distro habits are either ineffective or
destructive on NixOS; guidance that ignores this breaks on the next rebuild or
undermines the system's reproducibility guarantees.

### II. Bare-Metal Firmware, No RTOS (NON-NEGOTIABLE)

All nRF52832 firmware MUST be bare-metal C/C++ built on the nRF5 SDK 17.1.0 +
`arm-none-eabi-gcc` + OpenOCD toolchain. Zephyr application builds, FreeRTOS,
`nrf-bm` (nRF54L-only, incompatible with this board), or any other RTOS/scheduling
framework MUST NOT be introduced unless the user explicitly changes this principle.
Rationale: the thesis's core contribution is measurable ultra-low-power behavior
(sleep currents, peripheral power-gating, interrupt-driven wakes); RTOS scheduling
and subsystem abstractions obscure the energy story the thesis must demonstrate.

### III. Locked Hardware & Product Architecture

Component and product decisions recorded in `AGENTS.md`'s "Architecture decisions"
table (Raytac `MDBT42Q-512KV2` MCU, Waveshare 2.13" e-Paper, ST `LIS3DH` accelerometer,
`MCP73831` charger, `AP2112K-3.3` LDO, `AP-1205V-P1` buzzer, DCF-77 time sync, internal
RTC with no external DS3231) are locked. Superseded choices (STM32L0, SSD1306 OLED,
external DS3231, fully offline/no-BLE) MUST NOT be reintroduced without an explicit
user rollback. Any proposed deviation from a locked decision MUST be raised as a
question to the user (see Principle VI) rather than decided unilaterally by an agent.

### IV. Frequent, Descriptive Commit History

Work MUST be committed often, in small reviewable units, with descriptive
imperative-mood messages using thesis-area prefixes (`thesis:`, `hardware:`,
`firmware:`, `web:`, `docs:`). Unrelated concerns (LaTeX, PCB, firmware, docs) MUST
be split across separate commits rather than batched together. Agents MUST commit
as soon as a coherent unit of work lands rather than saving everything for the end
of a session, and MUST NOT force-push, amend commits they did not just create (or
that are already pushed), or alter git config without explicit user request.
Rationale: promotor guidance treats commit history as a primary source for
reconstructing the thesis's chronology, decisions, and what-was-tried narrative;
sparse or squashed history is a liability when writing the paper.

### V. Thesis Writing Fidelity to Institutional Rules

Thesis prose MUST follow `clock-project-latex/thesis-writing-rules.yaml` (a
machine-readable extract of Greber's *Zasady pisania prac dyplomowych*, wyd. VI) for
structure, wstęp/zakończenie content, citation style, and figure/table conventions.
Typography MUST follow the WIT deanery rules already encoded in
`clock-project-latex/settings.tex` (Times/`newtx` family, 14/13/12 pt headings,
single spacing) wherever they conflict with Greber's defaults. Explicit promotor
instructions override both sources. Rationale: the thesis is graded against these
institutional conventions; ad hoc formatting or citation styles risk grade penalties
independent of technical merit.

### VI. Mandatory Clarification via AskQuestion (NON-NEGOTIABLE)

When an agent operating in this repository through cursor-agent/cursor-cli is
blocked on a genuine decision that belongs to the user — an ambiguous requirement, a
choice between multiple valid approaches, or any point where proceeding further
would require guessing user intent — it MUST use the built-in AskQuestion tool to
collect a structured answer, rather than listing options as plain prose or silently
picking one. This requirement does not extend to purely informational questions or
to cases with an unambiguous, safely reversible default. Rationale: structured
questions produce an auditable, unambiguous decision trail the thesis chronology can
reference, and they prevent agents from making architecture or scope calls that are
the user's to make.

## Monorepo Structure & Scope Separation

The repository is organized as: `clock-project-latex/` (thesis LaTeX, git submodule),
`hardware/` (KiCad schematic/PCB, BOMs), `firmware/` (bare-metal C/C++), `web/`
(Web Bluetooth companion app, scaffold-only), and `temp-resources/` (university
reference material). Root holds shared docs (`AGENTS.md`, README), Nix
flakes/shells, and CI. Agents MUST keep these concerns separated: do not mix
unrelated thesis, hardware, and firmware edits in a single commit, and place new
BOMs or pin maps under `hardware/` (referenced from the thesis) rather than only in
chat output. `AGENTS.md` is the canonical, detailed operational reference for
product requirements, the full locked-architecture table, installed tooling, and
day-to-day workflow guidance; it MUST stay consistent with this constitution.

## Development Workflow & Tooling Standards

Agents MUST prefer tooling already installed on this NixOS host (`gcc-arm-embedded`,
`nrf5-sdk`, `openocd`, `gdb`, `nrfconnect`, etc. — see `AGENTS.md` for the current
inventory) before proposing new Nix packages or downloads. A dependency that must be
available system-wide (e.g. udev rules for a debug probe) MUST be proposed as a
change to `~/thinkpad-nixos`, not as a manual system edit. When debugging a missing
command, agents MUST check `which`/`type` and whether the shell is inside a
nix-shell/devShell before assuming a package is absent. Curl-bash installers,
`chmod +x` workarounds for missing dynamic loaders, and treating `/etc/nixos` as the
durable config location are anti-patterns and MUST be avoided.

## Governance

This constitution supersedes ad hoc conventions for this repository. `AGENTS.md`
remains the living, detailed operational companion (full tables, tool versions,
commit-message examples); where the two disagree, this constitution wins and
`AGENTS.md` MUST be updated to match as part of the same change.

**Amendments**: Propose changes via the `/speckit-constitution` workflow. Version
bumps follow semantic versioning: MAJOR for backward-incompatible principle removals
or redefinitions, MINOR for new principles or materially expanded guidance, PATCH
for wording/clarification fixes. Every amendment MUST update the Sync Impact Report
HTML comment at the top of this file and MUST check the dependent templates listed
there for needed edits.

**Compliance review**: Before `/speckit-plan` produces an implementation plan for any
feature in this repo, the plan's Constitution Check gate MUST verify the feature
does not violate Principles I–III (NixOS-native tooling, bare-metal firmware, locked
architecture). Any violation MUST be recorded and justified in that plan's
Complexity Tracking section or the feature scope MUST be revised.

**Version**: 1.0.0 | **Ratified**: 2026-07-26 | **Last Amended**: 2026-07-26
