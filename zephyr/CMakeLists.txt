# TinyUSB stack integration for Zephyr
#
# Copyright (c) 2026 Infineon Technologies AG,
# or an affiliate of Infineon Technologies AG.
#
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_TINYUSB)
  # The TinyUSB sources are provided by the 'tinyusb' west project imported by
  # this asset repo's west.yml. That project is checked out into the 'tinyusb'
  # subdirectory of this asset module (asset path: modules/lib/tinyusb-zephyr,
  # sources path: modules/lib/tinyusb-zephyr/tinyusb). ZEPHYR_CURRENT_MODULE_DIR
  # points at this asset, so descend into the tinyusb source tree.
  get_filename_component(TUSB_DIR ${ZEPHYR_CURRENT_MODULE_DIR}/tinyusb ABSOLUTE)

  if(NOT EXISTS ${TUSB_DIR}/src/CMakeLists.txt)
    message(FATAL_ERROR
      "TinyUSB sources not found at ${TUSB_DIR}. Run 'west update' so the "
      "'tinyusb' project is fetched alongside the ifz-zephyr-lib asset.")
  endif()

  # Reuse the upstream helper that lists the TinyUSB core/class sources
  # (controller drivers are added separately below).
  include(${TUSB_DIR}/src/CMakeLists.txt)
  tinyusb_sources_get(TINYUSB_CORE_SRC)

  zephyr_library_named(tinyusb)
  zephyr_library_sources(${TINYUSB_CORE_SRC})

  # TinyUSB public headers must be visible to the application and the
  # controller drivers; portable headers use includes relative to src/.
  zephyr_include_directories(
    ${TUSB_DIR}/src
    ${TUSB_DIR}/lib/networking
  )

  # Application supplied tusb_config.h location (optional).
  if(NOT "${CONFIG_TINYUSB_CONFIG_PATH}" STREQUAL "")
    zephyr_include_directories(${CONFIG_TINYUSB_CONFIG_PATH})
  endif()

  # OS abstraction layer -> Zephyr kernel.
  zephyr_compile_definitions(CFG_TUSB_OS=OPT_OS_ZEPHYR)

  # MCU / controller selection (CFG_TUSB_MCU) derived from Kconfig.
  zephyr_compile_definitions(CFG_TUSB_MCU=${CONFIG_TINYUSB_CFG_TUSB_MCU})

  if(CONFIG_TINYUSB_LOG_LEVEL GREATER 0)
    zephyr_compile_definitions(CFG_TUSB_DEBUG=${CONFIG_TINYUSB_LOG_LEVEL})
    if(CONFIG_TINYUSB_LOG_LEVEL EQUAL 4)
      # Level 4 disables always-inline so logs are not dropped.
      zephyr_compile_definitions(TU_ATTR_ALWAYS_INLINE=)
    endif()
  endif()

  # Controller port sources.
  if(CONFIG_TINYUSB_PORT_DWC2_IFX)
    zephyr_library_sources(
      ${TUSB_DIR}/src/portable/synopsys/dwc2/dcd_dwc2.c
      ${TUSB_DIR}/src/portable/synopsys/dwc2/hcd_dwc2.c
      ${TUSB_DIR}/src/portable/synopsys/dwc2/dwc2_common.c
    )

    # Zephyr-native controller integration: connects the USBHS IRQ
    if(CONFIG_TINYUSB_DWC2_IFX_IRQ_CONNECT)
      zephyr_library_sources(${CMAKE_CURRENT_LIST_DIR}/src/tinyusb_dwc2_ifx.c)
    endif()
  endif()
endif()
