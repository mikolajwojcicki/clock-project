## Why

The project records prototype parts and final PCB decisions, but it does not yet record which tools and hardware the user actually has. The supplied purchase conversation also contains useful corrections, such as replacing the high-quiescent-current regulator candidate and avoiding an unverified 5 V buzzer choice, so the project needs one traceable inventory and BOM reconciliation now.

## What Changes

- Record user-reported owned tools: 3D printers, digital calipers, computer, logic analyzer, multimeter, and nRF52 DK.
- Record the additional tools that the user reported buying from the earlier recommendation, while marking model and ownership as user-reported until verified.
- Reconcile the prototype BOM with the exact hardware already discussed in this project.
- Separate owned, user-reported purchased, optional, planned, and unconfirmed items.
- Preserve the rule that final PCB BOM items from the earlier conversation are not automatically marked as purchased.
- Add exact product links and source notes from `temp-resources/shit-to-buy.md` where they match current project decisions.
- Review the final PCB BOM for the proposed TPS7A0533 regulator, DCF power switch, connector parts, buzzer driver, buttons, SWD header, crystal, and passive values.
- Record the AP2112K regulator conflict as an architecture decision that requires explicit confirmation before changing locked project documentation.
- Record the AP-1205V-P1 voltage uncertainty and retain a low-power piezo option for prototype evaluation.
- Keep the existing breadboard guide aligned with the reconciled prototype parts and available tools.

## Capabilities

### New Capabilities

- `hardware-inventory-and-bom`: Track hardware ownership, tool availability, purchase confidence, source links, and planned versus unconfirmed prototype and PCB BOM items.

### Modified Capabilities

- `breadboard-hardware-prototype`: Update material and tool requirements so the assembly guide reflects the reconciled inventory and does not claim unverified parts are available.

## Impact

- Changes `hardware/bom/proto-bom.md`, `hardware/bom/pcb-bom.md`, and hardware documentation.
- May update `AGENTS.md` and `openspec/project-principles.md` only if the user-approved regulator or buzzer architecture changes.
- Adds no firmware dependency and does not change the bare-metal nRF52832 workflow.
- Does not purchase components, verify shipping, or claim ownership of final PCB parts without project evidence or explicit user confirmation.
