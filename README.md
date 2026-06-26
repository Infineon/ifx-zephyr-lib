# Infineon TinyUSB Zephyr Glue Module

This repository is the dedicated Zephyr glue layer for the `tinyusb` asset.

## Repository Role

This repo owns Zephyr module integration:

- `zephyr/module.yml`
- `zephyr/Kconfig`
- `zephyr/CMakeLists.txt`

It is intended to be pulled by `west` as a module repository.

## Separation of Responsibilities

- `ifx-zephyr-lib` (this repo): Zephyr module metadata and glue.
- `ifx-zephyr-sdk`: sample applications that consume modules.
