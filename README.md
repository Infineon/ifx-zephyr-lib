# Infineon Audio Front-End Zephyr Glue Module

This repository is the dedicated Zephyr glue layer for the `audio-front-end` asset.

## Repository Role

This repo owns Zephyr module integration:

- `zephyr/module.yml`
- `zephyr/Kconfig`
- `zephyr/CMakeLists.txt`
- `zephyr/zephyr.cmake`
- compatibility glue under `zephyr/include/` and `zephyr/cy_afe_basic_stubs.c`

It is intended to be pulled by `west` as a module repository.

## Separation of Responsibilities

- `ifx-zephyr-lib` (this repo): Zephyr module metadata and glue.
- `ifx-zephyr-sdk`: sample applications that consume modules.

## Recommended west integration

```yaml
- name: ifx-audio-front-end-glue
	remote: infineon-github
	repo-path: audio-front-end/ifx-zephyr-lib
	path: modules/lib/ifx-audio-front-end
	revision: <branch-or-sha>

- name: audio-front-end
	remote: infineon-github
	repo-path: audio-front-end
	path: modules/lib/third_party/audio-front-end
	revision: release-v1.0.2
```

## Build notes

Enable in app config:

```conf
CONFIG_AUDIOFE=y
```

Basic mode defaults are provided for bring-up, with speech enhancement disabled unless explicitly enabled.
