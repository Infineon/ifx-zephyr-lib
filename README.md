# Infineon Audio Voice Core Zephyr Glue Module

This repository is the dedicated Zephyr glue layer for the `audio-voice-core` asset.

## Repository Role

This repo owns Zephyr module integration for voice core:

- `zephyr/module.yml`
- `zephyr/Kconfig`
- `zephyr/CMakeLists.txt`
- `zephyr/zephyr.cmake`
- compatibility glue under `zephyr/include/` and `zephyr/src/`

It is intended to be pulled by `west` as a module repository.

## Separation of Responsibilities

- `ifx-zephyr-lib` (this repo): Zephyr module metadata and glue.
- `ifx-zephyr-sdk`: sample applications that consume modules.

## Recommended west integration

```yaml
- name: ifx-audio-voice-core-glue
	remote: infineon-github
	repo-path: audio-voice-core/ifx-zephyr-lib
	path: modules/lib/ifx-audio-voice-core
	revision: <branch-or-sha>

- name: audio-voice-core
	remote: infineon-github
	repo-path: audio-voice-core
	path: modules/lib/third_party/audio-voice-core
	revision: release-v2.0.0
```

## Build notes

Enable in app config:

```conf
CONFIG_AVC=y
```

Default settings target basic CM33 integration for bring-up and can be extended for CM55 and toolchain-specific library selection.

