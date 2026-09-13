## MODIFIED Requirements

### Requirement: Guide identifies required materials and safe preparation

The guide SHALL list every required component, tool, cable, and software prerequisite. It SHALL identify optional parts, owned parts, user-reported purchased items, unconfirmed items, parts that can differ by module revision, and electrical hazards before assembly begins. It SHALL use the reconciled inventory as its source for availability and SHALL never present an unverified item as owned.

#### Scenario: Beginner prepares the work area

- **WHEN** a beginner follows the preparation section
- **THEN** the beginner can identify the nRF52 DK, each peripheral, the breadboard rails, the required USB connection, and the parts that must remain disconnected during initial checks

#### Scenario: User lacks an optional power part

- **WHEN** the user does not have the TP4056 board or Li-Po battery
- **THEN** the guide clearly states that USB power from the nRF52 DK is sufficient for the first prototype and identifies which power experiments must be skipped

#### Scenario: Inventory marks a tool as user-reported

- **WHEN** a tool is listed as user-reported purchased but its exact model or presence is not verified
- **THEN** the guide labels it as user-reported and provides a safe alternative or states which test must wait for verification
