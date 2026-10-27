# SmartShed-Hi3861

[English](README.md) | 简体中文

用于 MQTT 遥测、环境感知与执行器控制的 Hi3861 大棚固件。

归档项目，保留用于参考。

## 项目概述

Smart Shed 2.0 将公共运行代码与可复用传感器、执行器任务分开，提供环境/RGB 板和土壤/风扇/水泵板两个目标。仓库包含应用组件、板级配置及 Paho MQTT 源码，需要兼容的 OpenHarmony 构建工作区。

## 技术栈与依赖

C、OpenHarmony Lite/Hi3861 接口、GN/Ninja 及随仓库提供的 Paho MQTT。按目标准备 OLED、光照与温湿度传感器、RGB 灯、土壤传感器、风扇或水泵；Wi-Fi 与代理参数在本地配置。

## 文件结构

- `applications/sample/wifi-iot/app/BUILD.gn` — 当前板型选择
- `applications/sample/wifi-iot/app/workspaces/smart_shed_2.0/` — 公共运行代码、模块与板型
- `third_party/pahomqtt/` — MQTT 客户端库
- `vendor/` — 板级和产品集成

## 快速上手

1. 将 `applications/`、`vendor/` 及所需 MQTT 库集成到匹配的 OpenHarmony Hi3861 源码工作区。
2. 在 `applications/sample/wifi-iot/app/BUILD.gn` 中保留环境板目标 `workspaces/smart_shed_2.0/boards/env_rgb_board:smart_shed_board_env_rgb`，或改为土壤板目标 `workspaces/smart_shed_2.0/boards/soil_actuator_board:smart_shed_board_soil_actuator`。
3. 核对选定板型的接线，在公共运行模块配置网络，并与 HarmonyOS 应用保持 MQTT 主题一致。
4. 在完整 OpenHarmony 工作区根目录执行 `hb set` 选择产品，运行 `hb build`，再通过开发板工具烧录固件。
