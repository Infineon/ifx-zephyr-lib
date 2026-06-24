# Copyright (c) 2026 Infineon Technologies AG
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_ML_TFLITE_MICRO_INFINEON)

  # Add include directories for TensorFlow Lite Micro
  zephyr_include_directories(
    ${ZEPHYR_CURRENT_MODULE_DIR}/mtb-ml-tflite-micro/COMPONENT_ML_TFLM/include
    ${ZEPHYR_CURRENT_MODULE_DIR}/mtb-ml-tflite-micro/COMPONENT_ML_TFLM/include/flatbuffers/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  )

  # Core Infineon libraries - using absolute path to Zephyr module location
  zephyr_include_directories(${ZEPHYR_BASE}/modules/hal_infineon/core-lib/include)

  # Infineon abstraction-rtos integration (required dependency)
  # Use the full asset (modules/hal/infineon/abstraction-rtos) which contains
  # cyabs_rtos.h. The Zephyr-internal copy (zephyr/modules/hal_infineon/abstraction-rtos)
  # is stripped and only provides COMPONENT_ZEPHYR implementation headers.
  zephyr_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/../abstraction-rtos/include)
  zephyr_include_directories(${ZEPHYR_BASE}/modules/hal_infineon/abstraction-rtos/include/COMPONENT_ZEPHYR)

  # Add Zephyr RTOS abstraction source following the official pattern
  zephyr_library_sources_ifdef(CONFIG_USE_INFINEON_ABSTRACTION_RTOS
          ${ZEPHYR_BASE}/modules/hal_infineon/abstraction-rtos/source/COMPONENT_ZEPHYR/cyabs_rtos_zephyr.c)

  # Add Zephyr-specific wrapper implementation (stub for testing)
  zephyr_library_sources(${CMAKE_CURRENT_SOURCE_DIR}/source/tflite_micro_zephyr_stub.cpp)

  # Precompiled library selection logic.
  # When CONFIG_TENSORFLOW_LITE_MICRO=y, the Zephyr source-built tflite-micro
  # module already provides the full TFLite runtime. Linking the Infineon
  # precompiled library on top would cause duplicate symbol errors.
  # Skip the precompiled library in that case — Zephyr's build provides all symbols.
  if(CONFIG_TENSORFLOW_LITE_MICRO)
    message(STATUS "ML: CONFIG_TENSORFLOW_LITE_MICRO=y detected — "
      "using Zephyr source-built TFLite runtime; skipping Infineon precompiled library.")
  else()
  # Link the precompiled TensorFlow Lite Micro library from COMPONENT_ML_TFLM.
  # The asset ships only .a archives (no source), so we select the correct
  # variant based on CPU core and FPU ABI, then link it directly.
  if(EXISTS ${ZEPHYR_CURRENT_MODULE_DIR}/mtb-ml-tflite-micro/COMPONENT_ML_TFLM)
    # Determine NPU sub-component directory.
    # COMPONENT_U55 (M55/Ethos-U55) has no FPU subdirectory — goes straight to
    # TOOLCHAIN_GCC_ARM. All M33 variants (NNLite2, CM33-only) have
    # COMPONENT_HARDFP / COMPONENT_SOFTFP subdirectories.
    if(CONFIG_CPU_CORTEX_M55)
      # U55: no FPU ABI sub-directory
      set(TFLM_LIB
        ${ZEPHYR_CURRENT_MODULE_DIR}/mtb-ml-tflite-micro/COMPONENT_ML_TFLM/COMPONENT_U55/TOOLCHAIN_GCC_ARM/libtensorflow-microlite.a
      )
    else()
      # M33 (and M4): select NPU variant then FPU ABI sub-directory
      if(CONFIG_ML_MIDDLEWARE_NNLITE)
        set(TFLM_COMPONENT_DIR COMPONENT_NNLITE2)
      elseif(CONFIG_CPU_CORTEX_M33)
        set(TFLM_COMPONENT_DIR COMPONENT_ML_CM33_ONLY)
      else()
        set(TFLM_COMPONENT_DIR COMPONENT_CM4)
      endif()

      if(CONFIG_FPU)
        set(TFLM_FPU_DIR COMPONENT_HARDFP)
      else()
        set(TFLM_FPU_DIR COMPONENT_SOFTFP)
      endif()

      set(TFLM_LIB
        ${ZEPHYR_CURRENT_MODULE_DIR}/mtb-ml-tflite-micro/COMPONENT_ML_TFLM/${TFLM_COMPONENT_DIR}/${TFLM_FPU_DIR}/TOOLCHAIN_GCC_ARM/libtensorflow-microlite.a
      )
    endif()

    if(EXISTS ${TFLM_LIB})
      message(STATUS "ML: Linking Infineon precompiled TFLite Micro: ${TFLM_LIB}")
      zephyr_link_libraries(${TFLM_LIB})
    else()
      message(WARNING "ML: Precompiled TFLite Micro library not found at ${TFLM_LIB}")
    endif()
  endif()
  endif() # CONFIG_TENSORFLOW_LITE_MICRO else

  # Define TensorFlow Lite Micro compile definitions
  # Note: Do NOT set TF_LITE_STATIC_MEMORY=1 when CONFIG_TENSORFLOW_LITE_MICRO is active
  # because Zephyr's own tflite-micro module compiles array.cc which requires the
  # dynamic allocation functions (TfLiteIntArrayCreate etc.) that are guarded by
  # #ifndef TF_LITE_STATIC_MEMORY in common.h.
  zephyr_compile_definitions(
      ENABLE_TFLITE_MICRO_SUPPORT=1
      CONFIG_USE_INFINEON_ABSTRACTION_RTOS=1
      TF_LITE_MCU=1
      TF_LITE_DISABLE_X86_NEON=1
  )
  if(NOT CONFIG_TENSORFLOW_LITE_MICRO)
    zephyr_compile_definitions(TF_LITE_STATIC_MEMORY=1)
    # The Infineon precompiled libraries were built with TF_LITE_STRIP_ERROR_STRINGS,
    # so MicroPrintf / VMicroPrintf are not present as real symbols — they are
    # preprocessed away in the library's object files. Any TU that includes
    # micro_mutable_op_resolver.h or similar headers must also see this define,
    # otherwise the template/inline code emits calls to MicroPrintf that the
    # linker cannot resolve against the precompiled .a.
    zephyr_compile_definitions(TF_LITE_STRIP_ERROR_STRINGS=1)
  endif()

endif()