# hardware/kicad

|Project|Role|
|---|---|
|`dk-perfboard-shield.*`|**Frozen v1.0-shield**: soldered perfboard shield for nRF52 DK (`PCA10040`). Thesis prototype.|
|`breadboard-prototype.*`|Archive of the breadboard-era schematic (v1.1). Do not edit.|

## Shield exports (regenerate from the schematic)

```bash
kicad-cli sch erc --format report -o erc-dk-perfboard-shield.rpt dk-perfboard-shield.kicad_sch
kicad-cli sch export netlist --format orcadpcb2 -o dk-perfboard-shield.net dk-perfboard-shield.kicad_sch
kicad-cli sch export pdf -o dk-perfboard-shield.pdf dk-perfboard-shield.kicad_sch
```

`dk-perfboard-shield.net` (OrcadPCB2) is the input for VeroRoute layout (`hardware/veroroute/`).

## ERC: 0 errors, 2 warnings (`erc-dk-perfboard-shield.rpt`)

|Warning|Reason kept|
|---|---|
|`isolated_pin_label` on `CLOCK_SEC_MARK` (P0.22)|Intentional test pad: a bench timing marker with no second pin on the shield.|
|`lib_symbol_mismatch` for `Q1` (2N3904)|Embedded symbol copy differs from the current stock KiCad library (library drift). The embedded copy has E=1, B=2, C=3, the pinout the netlist and layout use. Updating the symbol would hide the drift; check pins against the part when soldering.|

## Library paths

`sym-lib-table` points at the KiCad symbol libraries in the Nix store (same as the breadboard project). After a `nixos-rebuild` that changes the `kicad-symbols` hash, update the `/nix/store/...-kicad-symbols-*` paths.
