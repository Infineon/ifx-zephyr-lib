if(CONFIG_AUDIOFE)

  # Module layout may either expose the asset at module root or nested under
  # an extra repository directory (e.g. audio-front-end/audio-front-end).
  set(AFE_ASSET_DIR "${ZEPHYR_CURRENT_MODULE_DIR}")
  if(EXISTS "${ZEPHYR_CURRENT_MODULE_DIR}/audio-front-end/source")
    set(AFE_ASSET_DIR "${ZEPHYR_CURRENT_MODULE_DIR}/audio-front-end")
  endif()

  message(STATUS "AFE: basic mode integration enabled")

  # Zephyr compatibility layer headers - MUST come first for header override precedence.
  zephyr_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/zephyr/include)

  # AFE module headers.
  zephyr_include_directories(${AFE_ASSET_DIR}/include)
  zephyr_include_directories(${AFE_ASSET_DIR}/source/include)

  # Core compile definitions for middleware compatibility.
  zephyr_compile_definitions(
    ENABLE_AFE_MW_SUPPORT=1
    ENABLE_AUDIO_FRONT_END_LOGS=3
    CONFIG_USE_INFINEON_ABSTRACTION_RTOS=1
  )

  if(CONFIG_AUDIOFE_TUNING)
    zephyr_compile_definitions(CY_AFE_ENABLE_TUNING_FEATURE=1)
  endif()

  # AFE source compilation - support both CMakeLists.txt and direct source inclusion
  if(EXISTS "${AFE_ASSET_DIR}/source/CMakeLists.txt")
    # If the third-party module provides its own CMakeLists, use it
    add_subdirectory(${AFE_ASSET_DIR}/source source)
  else()
    # Otherwise, collect and add sources directly for Zephyr build
    file(GLOB AFE_ALL_SOURCES "${AFE_ASSET_DIR}/source/*.c")
    set(AFE_SOURCES)

    foreach(src ${AFE_ALL_SOURCES})
      get_filename_component(filename ${src} NAME)

      # Exclude tuning sources if tuning is disabled
      if(filename MATCHES "tuner_" AND NOT CONFIG_AUDIOFE_TUNING)
        continue()
      endif()

      # Exclude speech enhancement sources if speech enhancement is disabled
      if(filename MATCHES "speech_enh" AND NOT CONFIG_AUDIOFE_SPEECH_ENH)
        continue()
      endif()

      list(APPEND AFE_SOURCES ${src})
    endforeach()

    if(AFE_SOURCES)
      zephyr_library()
      zephyr_library_sources(${AFE_SOURCES})

      # When speech enhancement is disabled, add stub implementations
      if(NOT CONFIG_AUDIOFE_SPEECH_ENH)
        zephyr_library_sources(${ZEPHYR_CURRENT_MODULE_DIR}/zephyr/cy_afe_audio_speech_enh_stub.c)
        zephyr_library_sources(${ZEPHYR_CURRENT_MODULE_DIR}/zephyr/cy_afe_audio_bd_calc_stub.c)
        zephyr_library_sources(${ZEPHYR_CURRENT_MODULE_DIR}/zephyr/cy_afe_usb_settings_weak.c)
      endif()

      list(LENGTH AFE_SOURCES num_sources)
      message(STATUS "AFE: compiled ${num_sources} source files from asset")
    else()
      message(FATAL_ERROR "AFE: no source files selected for compilation at ${AFE_ASSET_DIR}/source")
    endif()
  endif()

endif()
