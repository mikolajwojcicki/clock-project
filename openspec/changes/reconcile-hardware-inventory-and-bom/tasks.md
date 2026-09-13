## 1. Create the inventory record

- [x] 1.1 Add `hardware/inventory/prototype-tools.md` and verify that it separates tools, prototype hardware, final PCB candidates, and optional future equipment
- [x] 1.2 Define the inventory states `Owned`, `User-reported purchased`, `Planned`, `Optional`, `Unconfirmed`, and `Rejected`; verify that each state has one clear meaning
- [x] 1.3 Record the user-reported tools: 3D printers, digital calipers, computer, logic analyzer, multimeter, and nRF52 DK; verify that each entry cites the supplied purchase conversation and marks unknown models as unverified
- [x] 1.4 Record the additional recommended tools as user-reported purchased without inventing exact models; verify that PPK2, bench supply, soldering tools, hot-air or hot-plate tools, Li-Po safety equipment, USB-C tester, and consumables have separate status entries
- [x] 1.5 Record available tools by project task, including assembly, firmware bring-up, signal debugging, enclosure work, and low-power measurement; verify that missing or unverified tools are visible

## 2. Reconcile prototype hardware

- [x] 2.1 Update `hardware/bom/proto-bom.md` with exact identities for the PCA10040, Waveshare 2.13inch HAT Rev 2.1, Adafruit LIS3DH product 2809, DCF-1060N-800, breadboard, jumpers, and driver parts; verify each identity against current project records
- [x] 2.2 Preserve owned prototype status from the current BOM while adding an evidence or source column; verify that no confirmed prototype item becomes unowned without explicit evidence
- [x] 2.3 Record the AP-1205V-P1 voltage and drive uncertainty and add the low-power piezo option as a comparison candidate; verify that the existing buzzer architecture remains unchanged
- [x] 2.4 Add links to the local DCF-77 manual, Adafruit LIS3DH product page, Waveshare documentation, and the supplied research conversation; verify that each link resolves to the intended source or is clearly marked as a local reference
- [x] 2.5 Review `hardware/docs/breadboard-prototype-guide.md` against the reconciled prototype BOM; verify that required tools and parts use the same ownership labels and exact product identities

## 3. Reconcile final PCB candidates

- [x] 3.1 Add status and source columns to `hardware/bom/pcb-bom.md` without changing locked component choices; verify that every final PCB row is marked locked, proposed, optional, or pending verification
- [x] 3.2 Record TPS7A0533 as a proposed low-quiescent-current alternative to AP2112K-3.3; verify that `AGENTS.md` and `openspec/project-principles.md` still retain AP2112K-3.3 as locked
- [x] 3.3 Record the proposed DCF power switch, USB-C receptacle, battery connector, buttons, SWD header, crystal, and passive values as planned or pending verification; verify that none is marked purchased only because a shop link exists
- [x] 3.4 Review the MCP73831 `RPROG` value against the selected battery charge rate; verify that the BOM flags the value for electrical review instead of silently accepting the research conversation value
- [x] 3.5 Add the supplied research source to final PCB review notes; verify that product links and MPNs remain traceable without claiming current stock or shipping

## 4. Validate architecture and documentation consistency

- [x] 4.1 Compare the reconciled BOM against `AGENTS.md` and `openspec/project-principles.md`; verify that no locked architecture choice changed without an explicit approval record
- [x] 4.2 Check all inventory and BOM status labels for consistent vocabulary; verify that `Owned`, `User-reported purchased`, `Planned`, `Optional`, `Unconfirmed`, and `Rejected` are used consistently
- [x] 4.3 Check all product links, MPNs, voltage notes, and package notes against the supplied source material; verify that uncertain facts have an uncertainty note
- [x] 4.4 Run `openspec validate --type change "reconcile-hardware-inventory-and-bom" --strict` and verify that the change artifacts pass validation
- [x] 4.5 Run `openspec validate --specs --strict` and verify that both the new inventory capability and the modified breadboard capability pass validation
