## MODIFIED Requirements

### Requirement: Generator builds and flashes with installed tools
The generator firmware SHALL build with the installed `arm-none-eabi-gcc` and the libopencm3 library pinned as a git submodule, and flash through the Nucleo's on-board ST-LINK with the installed OpenOCD, using its `board/st_nucleo_f4.cfg`. It SHALL NOT need STM32Cube, a vendor HAL, an RTOS, a NixOS change, or any download beyond initializing that submodule.

#### Scenario: Operator builds and flashes
- **WHEN** the operator initializes the submodule once, runs `make` and then `make flash` in the generator directory with the Nucleo connected over USB
- **THEN** the build completes without errors and OpenOCD programs, verifies, and resets the board

#### Scenario: No vendor tooling
- **WHEN** the generator is built on the NixOS host without STM32CubeMX, STM32CubeIDE or any ST package installed
- **THEN** the build still succeeds
