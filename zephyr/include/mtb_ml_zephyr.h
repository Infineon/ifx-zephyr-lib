/*
 * Copyright (c) 2026 Infineon Technologies AG
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file mtb_ml_zephyr.h
 * @brief Zephyr wrapper for Infineon ML Middleware
 *
 * This header provides a Zephyr-compatible interface to the Infineon ML Middleware
 * library. It includes the compatibility layer and the original ML middleware headers
 * in the correct order to ensure proper operation in a Zephyr environment.
 *
 * Application code should include this header instead of the individual ML
 * middleware headers to get full Zephyr integration.
 *
 * Example usage:
 * @code
 * #include "mtb_ml_zephyr.h"
 *
 * int main(void) {
 *     mtb_ml_model_t *model;
 *     cy_rslt_t result = mtb_ml_init();
 *     // ... rest of ML code
 * }
 * @endcode
 */

#ifndef MTB_ML_ZEPHYR_H
#define MTB_ML_ZEPHYR_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Pre-include Configuration
 ******************************************************************************/

/* Ensure Zephyr compatibility layer is included first */
#include "mtb_ml_zephyr_compat.h"

/* k_cycle_get_32() is in kernel.h; k_cyc_to_us_floor32() is in time_units.h
 * which kernel.h includes. Pull in kernel.h directly so both are visible
 * before the inline function bodies below are parsed. */
#include <zephyr/kernel.h>

/* Verify required Kconfig options */
#if !defined(CONFIG_ML_MIDDLEWARE_INFINEON)
#error "CONFIG_ML_MIDDLEWARE_INFINEON must be enabled to use ML Middleware"
#endif

#if !defined(CONFIG_ML_TFLITE_MICRO_INFINEON)
#error "CONFIG_ML_TFLITE_MICRO_INFINEON must be enabled for ML inference"
#endif

/*******************************************************************************
 * Original ML Middleware Headers (unmodified)
 ******************************************************************************/

/* Include original headers in dependency order */
#include "mtb_ml_common.h"
#include "mtb_ml_utils.h"
#include "mtb_ml_dataset.h"
#include "mtb_ml_model.h"

/* Optional streaming interface */
#ifdef CONFIG_ML_MIDDLEWARE_STREAMING
#include "mtb_ml_stream.h"
#endif

/* Main ML middleware header (includes others) */
#include "mtb_ml.h"

/*******************************************************************************
 * Zephyr-specific Extensions
 ******************************************************************************/

/**
 * @brief Zephyr-specific ML middleware initialization
 *
 * This function performs Zephyr-specific initialization in addition to
 * the standard ML middleware initialization. It should be called once
 * during application startup.
 *
 * @return CY_RSLT_SUCCESS on success, error code on failure
 */
cy_rslt_t mtb_ml_zephyr_init(void);

/**
 * @brief Get tensor arena size from Kconfig
 *
 * Returns the default tensor arena size configured via Kconfig.
 * Applications can use this to allocate appropriately sized buffers.
 *
 * @return Tensor arena size in bytes
 */
static inline size_t mtb_ml_get_default_tensor_arena_size(void)
{
    return CONFIG_ML_MIDDLEWARE_TENSOR_ARENA_SIZE;
}

/**
 * @brief Get maximum number of models from Kconfig
 *
 * Returns the maximum number of simultaneously loaded models
 * as configured via Kconfig.
 *
 * @return Maximum number of models
 */
static inline uint32_t mtb_ml_get_max_models(void)
{
    return CONFIG_ML_MIDDLEWARE_MAX_MODELS;
}

/**
 * @brief Check if NPU acceleration is available
 *
 * Returns true if any NPU acceleration (Ethos-U55 or NNLite) is
 * enabled and available on the current platform.
 *
 * @return true if NPU acceleration is available, false otherwise
 */
static inline bool mtb_ml_is_npu_available(void)
{
#if defined(CONFIG_ML_MIDDLEWARE_ETHOS_U55) || defined(CONFIG_ML_MIDDLEWARE_NNLITE)
    return true;
#else
    return false;
#endif
}

/**
 * @brief Get NPU type string
 *
 * Returns a human-readable string describing the available NPU type.
 *
 * @return String describing NPU type
 */
static inline const char* mtb_ml_get_npu_type(void)
{
    /* ML_MIDDLEWARE_ETHOS_U55 is set by the ML Middleware overlay and
     * indicates Ethos-U55 routed through the Infineon ML Middleware.
     * CONFIG_ETHOS_U is also set (needed to compile the ethosu TFLite
     * kernel) but CONFIG_ETHOS_U_INFINEON (the native Zephyr vendor driver)
     * is NOT set — check ML_MIDDLEWARE_ETHOS_U55 first. */
#if defined(CONFIG_ML_MIDDLEWARE_ETHOS_U55)
    return "ARM Ethos-U55 (Infineon ML Middleware)";
#elif defined(CONFIG_ML_MIDDLEWARE_NNLITE)
    return "Infineon NNLite";
#elif defined(CONFIG_ETHOS_U)
    return "ARM Ethos-U55 (Zephyr native driver)";
#else
    return "Software only";
#endif
}

/**
 * @brief Get quantization type string
 *
 * Returns a human-readable string describing the active quantization type.
 *
 * @return String describing quantization type
 */
static inline const char* mtb_ml_get_quantization_type(void)
{
#ifdef CONFIG_ML_MIDDLEWARE_FLOAT32
    return "32-bit Float";
#elif defined(CONFIG_ML_MIDDLEWARE_INT8)
    return "8-bit Integer";
#elif defined(CONFIG_ML_MIDDLEWARE_INT16)
    return "16-bit Integer";
#else
    return "Runtime detection";
#endif
}

/**
 * @brief Get inference engine string
 *
 * Returns a human-readable string describing the active inference engine.
 *
 * @return String describing inference engine
 */
static inline const char* mtb_ml_get_inference_engine(void)
{
#ifdef CONFIG_ML_MIDDLEWARE_TFLITE_INTERPRETER
    return "TensorFlow Lite with interpreter";
#elif defined(CONFIG_ML_MIDDLEWARE_TFLITE_LESS)
    return "TensorFlow Lite interpreter-less";
#else
    return "Unknown";
#endif
}

/*******************************************************************************
 * Performance Monitoring Extensions
 ******************************************************************************/

#ifdef CONFIG_ML_MIDDLEWARE_PERFORMANCE_MONITORING

/**
 * @brief Performance measurement structure
 */
typedef struct {
    uint32_t start_cycles;       /**< Start cycle count */
    uint32_t end_cycles;         /**< End cycle count */
    uint32_t duration_cycles;    /**< Duration in cycles */
    uint32_t duration_us;        /**< Duration in microseconds */
} mtb_ml_perf_t;

/**
 * @brief Start performance measurement
 *
 * @param perf Pointer to performance measurement structure
 */
static inline void mtb_ml_perf_start(mtb_ml_perf_t *perf)
{
    if (perf) {
        perf->start_cycles = k_cycle_get_32();
    }
}

/**
 * @brief End performance measurement and calculate duration
 *
 * @param perf Pointer to performance measurement structure
 */
static inline void mtb_ml_perf_end(mtb_ml_perf_t *perf)
{
    if (perf) {
        perf->end_cycles = k_cycle_get_32();
        perf->duration_cycles = perf->end_cycles - perf->start_cycles;
        perf->duration_us = k_cyc_to_us_floor32(perf->duration_cycles);
    }
}

#endif /* CONFIG_ML_MIDDLEWARE_PERFORMANCE_MONITORING */

#ifdef __cplusplus
}
#endif

#endif /* MTB_ML_ZEPHYR_H */