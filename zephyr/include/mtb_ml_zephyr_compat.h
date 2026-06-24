/*
 * Copyright (c) 2026 Infineon Technologies AG
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file mtb_ml_zephyr_compat.h
 * @brief Zephyr compatibility layer for Infineon ML Middleware
 *
 * This header provides compatibility mappings between the ML middleware's
 * Cypress PDL dependencies and Zephyr APIs. It leverages the existing
 * Infineon RTOS abstraction layer to maintain consistency across modules.
 *
 * This file should be included before any ML middleware headers to ensure
 * proper API mapping without modifying the original source code.
 *
 * NOTE: We do NOT wrap this in extern "C" because it gets force-included
 * into both C and C++ files that contain templates. Each header included
 * here handles its own linkage appropriately.
 */

#ifndef MTB_ML_ZEPHYR_COMPAT_H
#define MTB_ML_ZEPHYR_COMPAT_H

/*
 * CRITICAL: Skip this entire header for assembly files
 * This compatibility header is force-included via -include flag for all files,
 * but assembly files cannot parse C/C++ constructs like typedefs, includes, etc.
 */
#ifndef __ASSEMBLER__

/* Handle potential macro redefinition warnings from Infineon HAL */
#ifdef COMPONENT_SECURE_DEVICE
#undef COMPONENT_SECURE_DEVICE
#endif

/*
 * IMPORTANT: No Zephyr headers are included here.
 *
 * This header is force-included into every translation unit via
 * zephyr_compile_options(-include ...), which propagates through
 * zephyr_interface to ALL Zephyr libraries — including Zephyr's own
 * build-infrastructure libraries such as lib/heap/heap_constants.
 *
 * Since Zephyr 4.4, <zephyr/kernel.h> includes <zephyr/heap_constants.h>,
 * a generated header produced by compiling heap_constants.c itself.
 * Any Zephyr header that transitively reaches kernel.h therefore creates
 * an unsolvable circular dependency when force-included into heap_constants.c:
 *
 *   heap_constants.c
 *     → (force-include) mtb_ml_zephyr_compat.h
 *     → <zephyr/sys/util.h> | cyabs_rtos.h | <zephyr/kernel.h> | …
 *     → <zephyr/kernel.h>
 *     → <zephyr/heap_constants.h>   ← does not exist yet!
 *
 * Middleware source files obtain Zephyr headers through their own
 * explicit includes; no Zephyr header is needed in this compat shim.
 *   mtb_ml_rtos.c    → cyabs_rtos.h → cyabs_rtos_impl.h → <zephyr/kernel.h>
 *   mtb_ml_zephyr.c  → <zephyr/init.h>, <zephyr/logging/log.h>
 *   mtb_ml_model.cpp → TFLite headers (which bring in what they need)
 */

/* Standard C library mappings */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

/*
 * Map Zephyr Kconfig SoC/CPU symbols to the bare PDL macros expected by
 * Infineon device headers (e.g. cy_device_headers.h checks for PSE846GPS2DBZC4A).
 * infineon_kconfig.h contains only #define/#undef directives driven by
 * CONFIG_* symbols; it has no #include of any Zephyr or system headers, so it
 * is safe to pull in from this force-included shim.
 */
#include <infineon_kconfig.h>

/*
 * Pull in the device header so that CMSIS core_cm55.h (and the cache
 * maintenance functions it transitively includes via armv7m_cachel1.h) are
 * available to middleware TUs that call SCB_CleanDCache_by_Addr /
 * SCB_InvalidateDCache_by_Addr.
 *
 * Why this is safe: cy_device_headers.h and pse846gps2dbzc4a.h are already
 * included via soc.h → cy_device_headers.h in every Zephyr kernel / driver
 * TU; both files carry include guards so the second inclusion is a no-op.
 * Neither cy_device_headers.h nor core_cm55.h pulls in <zephyr/kernel.h>,
 * so the heap_constants.h circular-dependency risk described above does not
 * apply.  The PSE846GPS2DBZC4A / COMPONENT_CM55 macros required to select
 * the correct branch are already defined by the infineon_kconfig.h include
 * above.
 */
#include <cy_device_headers.h>

/*******************************************************************************
 * MTB Component Definitions
 * Define necessary component flags to enable correct struct definitions
 ******************************************************************************/

/* Enable TensorFlow Lite Micro support in MTB structures */
#ifndef COMPONENT_ML_TFLM
#define COMPONENT_ML_TFLM
#endif

/* Define float32 support for MTB middleware when enabled */
#ifdef CONFIG_ML_MIDDLEWARE_FLOAT32
#ifndef COMPONENT_ML_FLOAT32
#define COMPONENT_ML_FLOAT32
#endif
#endif

/* Use proper C/C++ linkage for our definitions */
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Cypress Result Framework Compatibility
 ******************************************************************************/

/* Use the existing cy_rslt_t from core-lib via cyabs_rtos.h - no redefinition needed */
/* Additional result codes for ML module */
#define CY_RSLT_MODULE_ML_ERROR        (-2000)

/*******************************************************************************
 * ML Data Type Compatibility
 ******************************************************************************/

/* Define ML data type used throughout the middleware */
#if defined(CONFIG_ML_MIDDLEWARE_INT8) || defined(COMPONENT_ML_INT8)
    typedef int8_t MTB_ML_DATA_T;
#elif defined(CONFIG_ML_MIDDLEWARE_INT16) || defined(COMPONENT_ML_INT16) || defined(COMPONENT_ML_INT16X8)
    typedef int16_t MTB_ML_DATA_T;
#elif defined(CONFIG_ML_MIDDLEWARE_FLOAT32) || defined(COMPONENT_ML_FLOAT32)
    typedef float MTB_ML_DATA_T;
#else
    /* Default to float for compatibility */
    typedef float MTB_ML_DATA_T;
#endif

#ifdef __cplusplus
}
#endif

/*******************************************************************************
 * Standard Library Compatibility for C++
 *
 * When -nostdinc++ is used, standard C++ headers like <climits> are not
 * available. Provide essential constants that might be needed by TensorFlow
 * Lite Micro or other C++ components.
 ******************************************************************************/
#ifdef __cplusplus

/* Standard integer limits from <climits> */
#ifndef INT_MAX
#define INT_MAX 2147483647
#endif
#ifndef INT_MIN
#define INT_MIN (-INT_MAX - 1)
#endif
#ifndef UINT_MAX
#define UINT_MAX 4294967295U
#endif
#ifndef CHAR_MAX
#define CHAR_MAX 127
#endif
#ifndef CHAR_MIN
#define CHAR_MIN (-128)
#endif
#ifndef LONG_MAX
#define LONG_MAX 2147483647L
#endif
#ifndef LONG_MIN
#define LONG_MIN (-LONG_MAX - 1L)
#endif
#ifndef ULONG_MAX
#define ULONG_MAX 4294967295UL
#endif
#ifndef LLONG_MAX
#define LLONG_MAX 9223372036854775807LL
#endif
#ifndef LLONG_MIN
#define LLONG_MIN (-LLONG_MAX - 1LL)
#endif
#ifndef ULLONG_MAX
#define ULLONG_MAX 18446744073709551615ULL
#endif

/* Size limits from <cstdint> / <stdint.h> */
#ifndef SIZE_MAX
#define SIZE_MAX UINT_MAX
#endif
#ifndef PTRDIFF_MAX
#define PTRDIFF_MAX INT_MAX
#endif
#ifndef PTRDIFF_MIN
#define PTRDIFF_MIN INT_MIN
#endif

/* Boolean constants from <cstdbool> */
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

#endif /* __cplusplus */

#endif /* __ASSEMBLER__ - End of C/C++ only content */

#endif /* MTB_ML_ZEPHYR_COMPAT_H */