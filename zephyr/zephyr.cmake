if(CONFIG_AVC)

  set(AVC_SOURCE_DIR "${CONFIG_AVC_SOURCE_DIR}")

  zephyr_library_named(ifx_audio_voice_core_glue)

  # Voice-core module local compatibility headers.
  zephyr_library_include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

  # Upstream voice-core include roots.
  if(EXISTS "${AVC_SOURCE_DIR}/lib/SP_ENH/COMPONENT_CM33/inc")
    zephyr_library_include_directories("${AVC_SOURCE_DIR}/lib/SP_ENH/COMPONENT_CM33/inc")
  endif()

  if(EXISTS "${AVC_SOURCE_DIR}/lib/SP_ENH/COMPONENT_CM33/src/inc")
    zephyr_library_include_directories("${AVC_SOURCE_DIR}/lib/SP_ENH/COMPONENT_CM33/src/inc")
  endif()

  if(EXISTS "${AVC_SOURCE_DIR}/lib/ASRC/COMPONENT_CM33/inc")
    zephyr_library_include_directories("${AVC_SOURCE_DIR}/lib/ASRC/COMPONENT_CM33/inc")
  endif()

  if(CONFIG_AVC_TARGET_CM55)
    if(EXISTS "${AVC_SOURCE_DIR}/lib/SP_ENH/COMPONENT_CM55/inc")
      zephyr_library_include_directories("${AVC_SOURCE_DIR}/lib/SP_ENH/COMPONENT_CM55/inc")
    endif()
    if(EXISTS "${AVC_SOURCE_DIR}/lib/SP_ENH/COMPONENT_CM55/src/inc")
      zephyr_library_include_directories("${AVC_SOURCE_DIR}/lib/SP_ENH/COMPONENT_CM55/src/inc")
    endif()
    if(EXISTS "${AVC_SOURCE_DIR}/lib/ASRC/COMPONENT_CM55/inc")
      zephyr_library_include_directories("${AVC_SOURCE_DIR}/lib/ASRC/COMPONENT_CM55/inc")
    endif()
  endif()

  zephyr_library_sources(src/ifx_audio_voice_core_glue.c)

  message(STATUS "AVC: using asset path ${AVC_SOURCE_DIR}")

endif()
