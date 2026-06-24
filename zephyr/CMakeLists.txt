# Copyright (c) 2026 Infineon Technologies AG
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_ML_MIDDLEWARE_INFINEON)

  # Add include directories following the "Zephyr way" for hal_infineon
  # 1. Zephyr-specific compatibility layer headers FIRST
  zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

  # 2. Core ML middleware headers - using current module directory.
  # Use -isystem (in addition to -I) so GCC treats these as system/vendor
  # headers and does not emit macro-redefinition warnings.  Specifically,
  # mtb_ml_model_defs.h unconditionally re-#defines STRINGIFY and CONCAT
  # with definitions that differ from Zephyr's toolchain/common.h and
  # sys/util.h equivalents.  Making the include directory a system directory
  # suppresses those benign warnings without touching any source files.
  # zephyr_compile_options is safe here: -isystem does not pull in any
  # headers, so there is no heap_constants circular-dependency risk.
  zephyr_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/mtb-ml-middleware/include)
  zephyr_compile_options(-isystem "${ZEPHYR_CURRENT_MODULE_DIR}/mtb-ml-middleware/include")

  # Force include compatibility header into ml-middleware sources only.
  # Using zephyr_library_compile_options (not zephyr_compile_options) to avoid
  # propagating -include to every TU in the build.  Global force-inclusion of
  # kernel.h pulls in the generated zephyr/heap_constants.h, which does not
  # exist when the heap_constants OBJECT library compiles because that library
  # is a separate CMake target compiled before the zephyr ARM library sources.
  zephyr_library_compile_options(-include "${CMAKE_CURRENT_SOURCE_DIR}/include/mtb_ml_zephyr_compat.h")

  # 3. Core Infineon libraries - path relative to this module's directory
  zephyr_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/../core-lib/include)

  # 4. Infineon abstraction-rtos integration (required dependency)
  # Use the full asset (modules/hal/infineon/abstraction-rtos) which contains
  # cyabs_rtos.h. The Zephyr-internal copy (zephyr/modules/hal_infineon/abstraction-rtos)
  # is stripped and only provides COMPONENT_ZEPHYR implementation headers.
  zephyr_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/../abstraction-rtos/include)
  zephyr_include_directories(${ZEPHYR_BASE}/modules/hal_infineon/abstraction-rtos/include/COMPONENT_ZEPHYR)

  # Add Zephyr RTOS abstraction source following the official pattern
  zephyr_library_sources_ifdef(CONFIG_USE_INFINEON_ABSTRACTION_RTOS
          ${ZEPHYR_BASE}/modules/hal_infineon/abstraction-rtos/source/COMPONENT_ZEPHYR/cyabs_rtos_zephyr.c)

  # Add Zephyr-specific implementation
  zephyr_library_sources(${CMAKE_CURRENT_SOURCE_DIR}/source/mtb_ml_zephyr.c)

  # Add core ML middleware source files
  zephyr_library_sources(
    ${ZEPHYR_CURRENT_MODULE_DIR}/mtb-ml-middleware/source/mtb_ml.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/mtb-ml-middleware/source/mtb_ml_utils.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/mtb-ml-middleware/source/mtb_ml_rtos.c
  )

  # NNLite NPU support: compile PDL kernel + nnkernel ops against the
  # COMPONENT_NNLITE2 precompiled libtensorflow-microlite.a
  if(CONFIG_ML_MIDDLEWARE_NNLITE)
    set(NNKERNEL_DIR ${ZEPHYR_CURRENT_MODULE_DIR}/../mtb-dsl-pse8xxgp/nnkernel/COMPONENT_CM33)
    set(DSL_PDL_DRV_DIR ${ZEPHYR_CURRENT_MODULE_DIR}/../mtb-dsl-pse8xxgp/pdl/drivers)

    zephyr_include_directories(${NNKERNEL_DIR}/include)

    # ML middleware NNLite glue (mtb_ml_nnlite_init / mtb_ml_nnlite_deinit)
    zephyr_library_sources(${ZEPHYR_CURRENT_MODULE_DIR}/mtb-ml-middleware/source/mtb_ml_nnlite.c)

    # High-level Cy_NNLite_* op wrappers (gated on CY_IP_MXNNLITE_VERSION == 2)
    zephyr_library_sources(
      ${NNKERNEL_DIR}/source/cy_nn_kernel.c
      ${NNKERNEL_DIR}/source/cy_nn_dma_helper.c
      ${NNKERNEL_DIR}/source/cy_nn_lstm_calculate_gate_s8_s16.c
      ${NNKERNEL_DIR}/source/cy_nn_lstm_step_s8_s16.c
      ${NNKERNEL_DIR}/source/cy_nn_lstm_unidirectional_s8_s16.c
      ${NNKERNEL_DIR}/source/cy_nn_lstm_update_cell_state_s16.c
      ${NNKERNEL_DIR}/source/cy_nn_lstm_update_output_s8_s16.c
    )

    # PDL NNLite driver: Cy_NNLite_Init, Cy_NNLite_InterruptHandler, etc.
    zephyr_library_sources(${DSL_PDL_DRV_DIR}/source/cy_nnlite.c)

    # DMA driver required by cy_nn_kernel.c (CY_NNLITE_DMA_DEVICE=1 by default).
    # Only added if not already compiled by CONFIG_USE_INFINEON_DMA.
    zephyr_library_sources_ifndef(CONFIG_USE_INFINEON_DMA
      ${DSL_PDL_DRV_DIR}/source/cy_dma.c)

    zephyr_compile_definitions(COMPONENT_NNLITE2=1)
  endif() # CONFIG_ML_MIDDLEWARE_NNLITE

  # Define required ML middleware support macros
  zephyr_compile_definitions(
    ENABLE_ML_MW_SUPPORT=1
    CONFIG_USE_INFINEON_ABSTRACTION_RTOS=1
  )

  # Add quantization-specific compile definitions
  if(CONFIG_ML_MIDDLEWARE_FLOAT32)
    zephyr_compile_definitions(COMPONENT_ML_FLOAT32=1)
  elseif(CONFIG_ML_MIDDLEWARE_INT8)
    zephyr_compile_definitions(COMPONENT_ML_INT8x8=1)
  elseif(CONFIG_ML_MIDDLEWARE_INT16)
    zephyr_compile_definitions(COMPONENT_ML_INT16x8=1)
  endif()

  # Add inference engine specific sources and definitions
  if(CONFIG_ML_MIDDLEWARE_TFLITE_INTERPRETER)

    # Use Zephyr-native implementation (no Infineon TFLite dependency)
    if(CONFIG_ML_MIDDLEWARE_ZEPHYR_BACKEND)
      message(STATUS "ML: Using Zephyr-native TensorFlow Lite backend")
      add_subdirectory(${ZEPHYR_CURRENT_MODULE_DIR}/mtb-ml-middleware/source/COMPONENT_ML_ZEPHYR ml_zephyr)
      zephyr_library_sources(
        ${ZEPHYR_CURRENT_MODULE_DIR}/mtb-ml-middleware/source/COMPONENT_ML_ZEPHYR/mtb_ml_model.c
      )
      zephyr_compile_definitions(
        COMPONENT_ML_ZEPHYR=1
        ZEPHYR_TFLITE_BACKEND=1
      )
    else()
      # Original Infineon TFLite implementation using real mtb_ml_model.cpp
      # Add TFLM component include dir so mtb_ml_model_defs.h resolves correctly
      zephyr_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/mtb-ml-middleware/source/COMPONENT_ML_TFLM)
      zephyr_library_sources(
        ${ZEPHYR_CURRENT_MODULE_DIR}/mtb-ml-middleware/source/COMPONENT_ML_TFLM/mtb_ml_model.cpp
      )
      # Suppress -Wdouble-promotion for all C++ files compiled into the zephyr
      # library (mtb_ml_model.cpp and ethosu_tflm_kernel.cc).  Both are
      # vendor-originated files that call printf() with (float) casts, which
      # GCC promotes to double under default argument promotion rules.
      # set_source_files_properties() does not work for files outside the
      # current CMake directory scope in CMake 3.25; the generator-expression
      # form of zephyr_library_compile_options() is the correct alternative.
      zephyr_library_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-double-promotion>)
      zephyr_compile_definitions(
        COMPONENT_ML_TFLM=1
      )

      # Ethos-U55 via ML Middleware: compile the Ethos-U driver glue and define
      # COMPONENT_U55 so mtb_ml_model.cpp wires the ethosu_driver handle into
      # the TFLite interpreter, enabling CUSTOM op dispatch to the NPU.
      if(CONFIG_ML_MIDDLEWARE_ETHOS_U55)
        # Use the Infineon-compatible ethos-u-core-driver from mtb-dsl-pse8xxgp.
        # The Zephyr hal/ethos_u module has a newer API that is incompatible with
        # the Infineon ML Middleware and precompiled libtensorflow-microlite.a:
        #   - hal/ethos_u: ethosu_flush_dcache(const uint64_t*, const size_t*, int)
        #   - Infineon:    ethosu_flush_dcache(uint32_t*, size_t)  [weak callback]
        # mtb_ml_ethosu.c provides the weak-override implementations of
        # ethosu_flush_dcache and ethosu_invalidate_dcache using the Infineon API.
        set(ETHOSU_IFX_DIR ${ZEPHYR_CURRENT_MODULE_DIR}/../mtb-dsl-pse8xxgp/pdl/drivers/third_party/COMPONENT_U55)
        zephyr_library_sources(
          ${ETHOSU_IFX_DIR}/src/ethosu_driver.c
          ${ETHOSU_IFX_DIR}/src/ethosu_pmu.c
          ${ETHOSU_IFX_DIR}/src/ethosu_device_u55_u65.c
          ${ZEPHYR_CURRENT_MODULE_DIR}/mtb-ml-middleware/source/COMPONENT_U55/mtb_ml_ethosu.c
          # TFLite Micro CUSTOM op kernel for Ethos-U — provides Register_ETHOSU()
          # and GetString_ETHOSU() so AllOpsResolver::AddEthosU() resolves at link
          # time. Compiled here instead of via CONFIG_ETHOS_U (which would pull in
          # the native Zephyr Ethos-U vendor driver conflicting with ML Middleware).
          ${CMAKE_CURRENT_SOURCE_DIR}/source/ethosu_tflm_kernel.cc
        )
        zephyr_include_directories(
          ${ZEPHYR_CURRENT_MODULE_DIR}/mtb-ml-middleware/source/COMPONENT_U55
          # Infineon-bundled ethos-u-core-driver headers (old API, matching precompiled lib).
          # Must come before any other ethosu_driver.h provider so mtb_ml.h resolves
          # to the correct API.
          ${ZEPHYR_CURRENT_MODULE_DIR}/../ml-tflite-micro/COMPONENT_ML_TFLM/include/ethos-u-core-driver
          ${ETHOSU_IFX_DIR}/src
          ${ETHOSU_IFX_DIR}/include
          ${ZEPHYR_BASE}/../modules/hal/cmsis_6/CMSIS/Core/Include
        )
        # The Infineon ethosu_driver.h includes cy_device.h → cy_device_headers.h
        # → pse846gps2dbzc4a.h, which requires three project-level macros that the
        # Infineon hal module sets on its own library target but NOT globally.
        # Because COMPONENT_U55=1 is a global zephyr_compile_definitions, every TU
        # (including app files like model.c) includes ethosu_driver.h and hits the
        # same requirement. Derive all three from Kconfig and set them globally here.
        #
        #   PSE846GPS2DBZC4A — from CONFIG_SOC (uppercase); selects device header
        #   COMPONENT_CM55   — from CONFIG_CPU_CORTEX_M55; selects core variant
        #   CORE_NAME_CM55_0 — companion to COMPONENT_CM55; fills IRQn_Type enum
        string(TOUPPER "${CONFIG_SOC}" _ETHOSU_SOC_UPPER)
        if(CONFIG_CPU_CORTEX_M55)
          set(_ETHOSU_CORE_DEFINES COMPONENT_CM55 CORE_NAME_CM55_0)
        elseif(CONFIG_CPU_CORTEX_M33)
          set(_ETHOSU_CORE_DEFINES COMPONENT_CM33 CORE_NAME_CM33_0)
        endif()
        zephyr_compile_definitions(
          COMPONENT_U55=1
          ETHOSU55=1     # pmu_ethosu.h: selects U55 PMU event set
          ETHOSU_ARCH=u55
          ETHOSU_MACS=256
          ETHOSU         # required by ethosu_driver internal headers
          ${_ETHOSU_SOC_UPPER}   # e.g. PSE846GPS2DBZC4A — satisfies cy_device_headers.h
          ${_ETHOSU_CORE_DEFINES} # COMPONENT_CM55 + CORE_NAME_CM55_0 — fills IRQn_Type
        )

        # Source-based TFLite Micro (CONFIG_TENSORFLOW_LITE_MICRO=y) has two
        # issues that must be patched here because the zephyr/modules/tflite-micro
        # CMakeLists cannot be modified:
        #
        # (a) Duplicate Register_ETHOSU(): The tflite-micro module unconditionally
        #     compiles kernels/ethosu.cc (stub, returns nullptr) regardless of
        #     CONFIG_ETHOS_U.  Our ethosu_tflm_kernel.cc provides the real
        #     Ethos-U dispatch  and is compiled into libzephyr.a, which the linker
        #     processes BEFORE libmodules__tflite-micro.a.  With
        #     --allow-multiple-definition GCC/BFD keeps the first definition
        #     (ours) and silently discards the duplicate stub.
        #
        # (b) Missing batch_matmul_common.cc: The tflite-micro module compiles
        #     batch_matmul.cc but omits batch_matmul_common.cc, leaving
        #     kBatchMatmulInputLhsTensor, ReshapeOutputTensor,
        #     TransposeRowsColumns, and SwapRowColumnDims undefined.  Compile
        #     batch_matmul_common.cc directly from the tflite-micro source tree.
        if(CONFIG_TENSORFLOW_LITE_MICRO)
          set(TFLM_KERNEL_DIR
            ${ZEPHYR_BASE}/../modules/lib/tflite-micro/tensorflow/lite/micro/kernels)
          if(EXISTS ${TFLM_KERNEL_DIR}/batch_matmul_common.cc)
            zephyr_library_sources(${TFLM_KERNEL_DIR}/batch_matmul_common.cc)
          endif()
          zephyr_ld_options(-Wl,--allow-multiple-definition)
        endif()

      endif()
    endif()

  endif()

endif() # CONFIG_ML_MIDDLEWARE_INFINEON