# Copyright (c) 2026 Infineon Technologies AG,
# or an affiliate of Infineon Technologies AG.
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_MWUTILS)

  # Module layout may either expose the asset at module root or nested under
  # an extra repository directory (e.g. ifx-connectivity-utilities/connectivity-utilities).
  set(MWUTILS_ASSET_DIR "${ZEPHYR_CURRENT_MODULE_DIR}")
  if(EXISTS "${ZEPHYR_CURRENT_MODULE_DIR}/connectivity-utilities")
    set(MWUTILS_ASSET_DIR "${ZEPHYR_CURRENT_MODULE_DIR}/connectivity-utilities")
  endif()

  # Add include paths
  zephyr_include_directories(
    ${MWUTILS_ASSET_DIR}
    ${MWUTILS_ASSET_DIR}/cy_log
    ${MWUTILS_ASSET_DIR}/cy_string
    ${MWUTILS_ASSET_DIR}/JSON_parser
    ${MWUTILS_ASSET_DIR}/linked_list
    ${MWUTILS_ASSET_DIR}/network
    ${ZEPHYR_CURRENT_MODULE_DIR}/zephyr/include
  )

  # Include standard Infineon core-lib for cy_result.h
  set(CORELIB_INCLUDE_DIR "${ZEPHYR_CURRENT_MODULE_DIR}/../../hal/infineon/core-lib/include")
  if(EXISTS "${CORELIB_INCLUDE_DIR}")
    zephyr_include_directories(${CORELIB_INCLUDE_DIR})
  else()
    message(WARNING "MWUTILS: core-lib include not found at ${CORELIB_INCLUDE_DIR}")
  endif()

  # Core middleware utilities
  set(MWUTILS_SOURCES)

  # cy_log support
  if(CONFIG_MWUTILS_LOG)
    list(APPEND MWUTILS_SOURCES
      ${MWUTILS_ASSET_DIR}/cy_log/cy_log.c
    )
  endif()

  # String utilities
  if(CONFIG_MWUTILS_STRING)
    # Add cy_string sources if they exist
    file(GLOB CY_STRING_SOURCES "${MWUTILS_ASSET_DIR}/cy_string/*.c")
    if(CY_STRING_SOURCES)
      list(APPEND MWUTILS_SOURCES ${CY_STRING_SOURCES})
    endif()
  endif()

  # JSON parser
  if(CONFIG_MWUTILS_JSON)
    file(GLOB JSON_SOURCES "${MWUTILS_ASSET_DIR}/JSON_parser/*.c")
    if(JSON_SOURCES)
      list(APPEND MWUTILS_SOURCES ${JSON_SOURCES})
    endif()
  endif()

  # Linked list utilities
  if(CONFIG_MWUTILS_LINKED_LIST)
    file(GLOB LINKED_LIST_SOURCES "${MWUTILS_ASSET_DIR}/linked_list/*.c")
    if(LINKED_LIST_SOURCES)
      list(APPEND MWUTILS_SOURCES ${LINKED_LIST_SOURCES})
    endif()
  endif()

  # Network utilities
  if(CONFIG_MWUTILS_NETWORK)
    file(GLOB NETWORK_SOURCES "${MWUTILS_ASSET_DIR}/network/*.c")
    if(NETWORK_SOURCES)
      list(APPEND MWUTILS_SOURCES ${NETWORK_SOURCES})
    endif()
  endif()

  # Create library if we have sources
  if(MWUTILS_SOURCES)
    zephyr_library()
    zephyr_library_sources(${MWUTILS_SOURCES})

    # Add any necessary compile definitions
    zephyr_library_compile_definitions(
      CY_MW_UTILITIES_SUPPORT=1
    )
  endif()

endif()
