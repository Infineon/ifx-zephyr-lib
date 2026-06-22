# Copyright (c) 2026 Infineon Technologies AG,
# or an affiliate of Infineon Technologies AG.
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_MWUTILS)

  # Add include paths
  zephyr_include_directories(
    ${ZEPHYR_CURRENT_MODULE_DIR}
    ${ZEPHYR_CURRENT_MODULE_DIR}/cy_log
    ${ZEPHYR_CURRENT_MODULE_DIR}/cy_string
    ${ZEPHYR_CURRENT_MODULE_DIR}/JSON_parser
    ${ZEPHYR_CURRENT_MODULE_DIR}/linked_list
    ${ZEPHYR_CURRENT_MODULE_DIR}/network
    ${ZEPHYR_CURRENT_MODULE_DIR}/zephyr/include
  )
  
  # Include standard Infineon core-lib for cy_result.h
  zephyr_include_directories(${CMAKE_SOURCE_DIR}/modules/hal/infineon/core-lib/include)

  # Core middleware utilities
  set(MWUTILS_SOURCES)

  # cy_log support
  if(CONFIG_MWUTILS_LOG)
    list(APPEND MWUTILS_SOURCES
      ${ZEPHYR_CURRENT_MODULE_DIR}/cy_log/cy_log.c
    )
  endif()

  # String utilities
  if(CONFIG_MWUTILS_STRING)
    # Add cy_string sources if they exist
    file(GLOB CY_STRING_SOURCES "${ZEPHYR_CURRENT_MODULE_DIR}/cy_string/*.c")
    if(CY_STRING_SOURCES)
      list(APPEND MWUTILS_SOURCES ${CY_STRING_SOURCES})
    endif()
  endif()

  # JSON parser
  if(CONFIG_MWUTILS_JSON)
    file(GLOB JSON_SOURCES "${ZEPHYR_CURRENT_MODULE_DIR}/JSON_parser/*.c")
    if(JSON_SOURCES)
      list(APPEND MWUTILS_SOURCES ${JSON_SOURCES})
    endif()
  endif()

  # Linked list utilities
  if(CONFIG_MWUTILS_LINKED_LIST)
    file(GLOB LINKED_LIST_SOURCES "${ZEPHYR_CURRENT_MODULE_DIR}/linked_list/*.c")
    if(LINKED_LIST_SOURCES)
      list(APPEND MWUTILS_SOURCES ${LINKED_LIST_SOURCES})
    endif()
  endif()

  # Network utilities
  if(CONFIG_MWUTILS_NETWORK)
    file(GLOB NETWORK_SOURCES "${ZEPHYR_CURRENT_MODULE_DIR}/network/*.c")
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
