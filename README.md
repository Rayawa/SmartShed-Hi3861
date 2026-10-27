# SmartShed-Hi3861

English | [简体中文](README_zh.md)

Hi3861 greenhouse firmware for MQTT telemetry, environmental sensing and actuator control.

Archived project retained for reference.

## Overview

The Smart Shed 2.0 workspace separates shared runtime code from reusable sensor and actuator tasks. Two board targets cover environment/RGB lighting and soil/fan/pump control. The repository includes application components, board configuration and Paho MQTT sources, and needs a compatible OpenHarmony build workspace.

## Technology and Dependencies

C, OpenHarmony Lite/Hi3861 APIs, GN/Ninja and bundled Paho MQTT. Required hardware depends on the selected target: OLED, light and temperature/humidity sensors, RGB lighting, soil sensor, fan and pump. Configure Wi-Fi and broker settings locally.

## Repository Structure

- `applications/sample/wifi-iot/app/BUILD.gn` — active board selection
- `applications/sample/wifi-iot/app/workspaces/smart_shed_2.0/` — shared runtime, modules and board targets
- `third_party/pahomqtt/` — MQTT client library
- `vendor/` — board and product integration

## Quick Start

1. Integrate `applications/`, `vendor/` and the required MQTT library into a matching OpenHarmony Hi3861 source workspace.
2. In `applications/sample/wifi-iot/app/BUILD.gn`, keep `workspaces/smart_shed_2.0/boards/env_rgb_board:smart_shed_board_env_rgb` for board 1, or select `workspaces/smart_shed_2.0/boards/soil_actuator_board:smart_shed_board_soil_actuator` for board 2.
3. Review the selected board's wiring and configure network access in the shared runtime. Match MQTT topics with the HarmonyOS application.
4. At the complete OpenHarmony workspace root, select the product using `hb set`, run `hb build`, then flash the resulting firmware using the board's tooling.
