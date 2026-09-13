## Context

See `proposal.md` for the reason for this change. The repository currently has a prototype BOM, a production PCB BOM, a breadboard assembly guide, and locked architecture rules in `AGENTS.md` and `openspec/project-principles.md`.

The supplied purchase conversation contains two kinds of information. It identifies tools and prototype parts that the user reports owning or buying. It also proposes final PCB parts and corrections that are not proof of purchase and are not all approved architecture decisions.

## Goals / Non-Goals

**Goals:**

- Create one clear ownership vocabulary for tools, prototype parts, and PCB candidates.
- Preserve exact product identity where the current project already confirmed it.
- Keep user-reported purchases separate from verified repository evidence.
- Record source links and technical warnings without silently changing locked architecture.
- Make the breadboard guide depend on a truthful availability record.

**Non-Goals:**

- Confirm physical possession without a user statement, photo, invoice, or existing project record.
- Treat every item in the research conversation as purchased.
- Replace AP2112K-3.3, AP-1205V-P1, or another locked choice without explicit approval.
- Redesign the schematic or PCB.
- Research new products or verify current shop stock as part of this change.

## Decisions

### Use explicit inventory states

Use these states:

- `Owned`: current project record or direct user statement confirms possession.
- `User-reported purchased`: the user reports buying the item, but exact model or physical presence is not verified.
- `Planned`: project intends to use the item, but purchase is not confirmed.
- `Optional`: useful for later work and not required for the current prototype.
- `Unconfirmed`: a source mentions the item, but the project cannot establish its status or compatibility.
- `Rejected`: the project has a reason not to use the item.

This vocabulary prevents a research list from becoming an accidental purchase record. A simple `yes` or `no` field cannot represent the difference between a locked choice and a candidate replacement.

### Keep separate inventories

Maintain separate sections for:

1. Tools and measurement equipment.
2. Prototype hardware.
3. Final PCB BOM.
4. Optional future equipment.

Keep exact MPNs and purchase links in the final PCB section. Keep physical ownership status in the inventory section. This avoids marking a PCB candidate as owned only because it has a distributor link.

### Treat prior project records as evidence for discussed prototype parts

The current prototype BOM and the user's project statements are valid evidence for prototype parts already discussed in this project. The guide will retain the exact confirmed hardware: PCA10040, Waveshare 2.13inch HAT Rev 2.1, Adafruit LIS3DH product 2809, DCF-1060N-800, breadboard, jumpers, and the previously listed buzzer driver parts.

The final PCB candidates from the research conversation remain planned or unconfirmed unless the current project already locked them. This applies to parts such as TPS7A0533, TPS22917, battery connectors, USB-C receptacle, buttons, and passives.

### Preserve locked architecture until explicit approval

The research conversation proposes replacing AP2112K-3.3 with TPS7A0533 because of quiescent current. The reconciliation will record this as a proposed change and explain its power reason. It will not edit the locked architecture files to replace AP2112K-3.3 during this change.

The same rule applies to AP-1205V-P1. The inventory will record its voltage uncertainty and a low-power piezo candidate. It will not silently replace the buzzer in the product architecture.

### Keep source notes close to decisions

Each correction will name its source:

- `temp-resources/shit-to-buy.md` for the supplied conversation.
- `hardware/bom/proto-bom.md` for current prototype ownership.
- `hardware/bom/pcb-bom.md` for current production candidates.
- Product manuals or datasheets when a value requires electrical confirmation.

This makes later thesis decisions traceable and prevents unsupported values from becoming design facts.

## Risks / Trade-offs

- [User-reported tool models are unknown] → Record the purchase claim without inventing model numbers. Mark model verification as pending.
- [Owned prototype BOM is stale] → Preserve its source and add a reconciliation note instead of silently deleting entries.
- [Research recommends a better regulator] → Record the proposal, but require explicit architecture approval before changing `AGENTS.md`, principles, or PCB BOM locks.
- [Research links become unavailable] → Keep the MPN and source conversation reference. Do not treat a dead link as proof that a component is unavailable.
- [A candidate part looks compatible but lacks a verified datasheet] → Mark it unconfirmed and block final BOM release until the datasheet is checked.
- [Tool inventory becomes too broad] → Separate prototype blockers from thesis-grade measurement upgrades and optional workshop equipment.

## Migration Plan

1. Add a hardware and tool inventory document under `hardware/`.
2. Update `hardware/bom/proto-bom.md` with exact prototype identities and ownership evidence.
3. Update `hardware/bom/pcb-bom.md` with status labels and source notes without changing locked choices.
4. Update `hardware/docs/breadboard-prototype-guide.md` only where its required materials or tool assumptions differ from the reconciled inventory.
5. Review all changes against `AGENTS.md` and `openspec/project-principles.md`.
6. Ask for explicit approval before changing the regulator or buzzer architecture.

Rollback removes the new inventory document and restores the previous BOM and guide text. No firmware or PCB design file changes are required for rollback.

## Open Questions

- Exact models of the additional tools reported as purchased remain unverified. The implementation can record them as user-reported without blocking the inventory.
- The final regulator choice remains open because changing it requires an explicit architecture decision. This change records the proposal but does not choose it.
