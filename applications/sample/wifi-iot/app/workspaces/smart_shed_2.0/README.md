## Smart Shed 2.0 Workspace Layout

This workspace is now split into:

- `common/`
  - Shared runtime, display driver, and GN build fragments.
- `modules/`
  - Reusable sensor and actuator task implementations.
- `boards/env_rgb_board/`
  - Board 1 target.
  - Modules: `oled + light_intensity + temp_and_hum + led`
- `boards/soil_actuator_board/`
  - Board 2 target.
  - Modules: `oled + soil_moisture + fan + water_pump`
- `legacy/`
  - Old single-module example projects kept only for reference.

Default app build target:

- `workspaces/smart_shed_2.0/boards/env_rgb_board:smart_shed_board_env_rgb`

To switch to board 2, edit:

- `applications/sample/wifi-iot/app/BUILD.gn`

Replace the board 1 target with:

- `workspaces/smart_shed_2.0/boards/soil_actuator_board:smart_shed_board_soil_actuator`

MQTT topic layout remains shared between the two boards.
