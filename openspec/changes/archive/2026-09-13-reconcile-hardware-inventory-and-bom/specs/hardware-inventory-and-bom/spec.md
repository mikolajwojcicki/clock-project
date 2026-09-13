## Purpose

This capability keeps project hardware records truthful by separating owned tools and prototype parts from planned, optional, user-reported, and unconfirmed BOM items.

## ADDED Requirements

### Requirement: Inventory records ownership confidence

The project SHALL record each tool and hardware item with an ownership state, evidence source, exact model when known, and a note when the item still needs user verification. User-reported ownership SHALL remain distinct from repository evidence.

#### Scenario: User reports buying recommended tools

- **WHEN** the user reports that recommended tools were bought
- **THEN** the inventory records them as user-reported purchased and does not present an exact model as verified unless the user or a project source identifies it

#### Scenario: Project source identifies an owned prototype part

- **WHEN** the current project BOM marks a prototype part as owned
- **THEN** the reconciled inventory preserves that ownership state and links to the BOM entry

#### Scenario: Item has no purchase evidence

- **WHEN** a final PCB item appears only in a research conversation or candidate BOM
- **THEN** the inventory marks it as planned or unconfirmed instead of owned

### Requirement: BOM separates prototype and production decisions

The project SHALL keep prototype hardware, production PCB hardware, tools, and optional future equipment in separate sections. Each item SHALL have a clear role and status.

#### Scenario: User builds the breadboard prototype

- **WHEN** the user checks the prototype BOM
- **THEN** the BOM identifies the exact nRF52 DK, Waveshare display, LIS3DH breakout, DCF-77 module, buzzer choice, driver parts, breadboard, and jumper wires required for the current prototype

#### Scenario: User reviews the final PCB BOM

- **WHEN** the user reviews a final PCB candidate
- **THEN** the BOM shows whether the item is locked, proposed, optional, or pending verification and does not imply that it was purchased

### Requirement: BOM corrections preserve decision traceability

The project SHALL record the source and reason for each BOM correction that affects power, compatibility, safety, or architecture. A proposed correction SHALL NOT silently replace a locked architecture choice.

#### Scenario: Low-quiescent-current regulator is proposed

- **WHEN** research proposes TPS7A0533 instead of AP2112K-3.3
- **THEN** the project records the lower-current regulator as a proposal, retains the current locked choice until explicit approval, and identifies the AGENTS.md and principle files that require updating after approval

#### Scenario: Buzzer voltage is uncertain

- **WHEN** the AP-1205V-P1 voltage or drive requirement is not confirmed by a suitable datasheet
- **THEN** the project marks the buzzer choice as requiring verification and records a lower-power piezo option for prototype comparison

#### Scenario: Charge current conflicts with battery rating

- **WHEN** a proposed MCP73831 `RPROG` value does not match the selected one-cell battery charge rate
- **THEN** the BOM flags the value for electrical review before PCB release and does not treat the research value as final

### Requirement: Tool inventory maps tools to project work

The project SHALL record which available tools support breadboard assembly, firmware bring-up, PCB assembly, signal debugging, enclosure work, and power measurement. It SHALL identify missing tools that block or limit a planned measurement.

#### Scenario: User plans low-power measurements

- **WHEN** the user plans sleep-current or BLE-current measurements
- **THEN** the inventory identifies Nordic Power Profiler Kit II or an equivalent as the measurement tool and distinguishes it from a multimeter or USB power meter

#### Scenario: User starts breadboard bring-up

- **WHEN** the user starts prototype wiring
- **THEN** the inventory identifies the nRF52 DK, breadboard, jumpers, multimeter, computer, and logic analyzer as available or user-reported tools and identifies any unverified item
