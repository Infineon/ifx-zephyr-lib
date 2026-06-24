/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2024 Infineon Technologies AG
 */

/**
 * @file tflite_micro_zephyr.h
 * @brief TensorFlow Lite Micro Zephyr Integration API
 * 
 * This header provides the main API for using TensorFlow Lite Micro within
 * Zephyr applications, with support for hardware acceleration and 
 * optimized memory management.
 */

#ifndef TFLITE_MICRO_ZEPHYR_H
#define TFLITE_MICRO_ZEPHYR_H

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>

/* Include compatibility layer */
#include "tflite_micro_zephyr_compat.h"

/* Include core TensorFlow Lite Micro headers */
#ifdef CONFIG_ML_TFLITE_MICRO_INFINEON

/* Conditional includes based on TensorFlow Lite Micro structure */
#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/* Core Types and Constants                                                   */
/* ========================================================================== */

/**
 * @brief TensorFlow Lite Micro result codes
 */
typedef enum {
    TFLITE_MICRO_SUCCESS = 0,          /**< Operation successful */
    TFLITE_MICRO_ERROR_GENERIC = -1,   /**< Generic error */
    TFLITE_MICRO_ERROR_MEMORY = -2,    /**< Memory allocation error */
    TFLITE_MICRO_ERROR_INVALID_ARG = -3, /**< Invalid argument */  
    TFLITE_MICRO_ERROR_NOT_INIT = -4,  /**< Not initialized */
    TFLITE_MICRO_ERROR_HARDWARE = -5,  /**< Hardware error */
    TFLITE_MICRO_ERROR_MODEL = -6,     /**< Model loading error */
    TFLITE_MICRO_ERROR_INFERENCE = -7  /**< Inference error */
} tflite_micro_result_t;

/**
 * @brief Hardware acceleration types
 */  
typedef enum {
    TFLITE_MICRO_ACCEL_NONE = 0,       /**< No hardware acceleration */
    TFLITE_MICRO_ACCEL_ETHOS_U55,      /**< ARM Ethos-U55 NPU */
    TFLITE_MICRO_ACCEL_NNLITE,         /**< Infineon NNLite NPU */
    TFLITE_MICRO_ACCEL_CMSIS_NN        /**< ARM CMSIS-NN optimization */
} tflite_micro_accel_type_t;

/**
 * @brief Model quantization types
 */
typedef enum {
    TFLITE_MICRO_QUANT_NONE = 0,       /**< No quantization */
    TFLITE_MICRO_QUANT_INT8,           /**< 8-bit integer quantization */
    TFLITE_MICRO_QUANT_INT16,          /**< 16-bit integer quantization */
    TFLITE_MICRO_QUANT_UINT8,          /**< 8-bit unsigned integer quantization */
    TFLITE_MICRO_QUANT_FLOAT32,        /**< 32-bit floating point */
    TFLITE_MICRO_QUANT_UNKNOWN         /**< Unknown quantization type */
} tflite_micro_quant_type_t;

/**
 * @brief Maximum number of tensor dimensions supported
 */
#define TFLITE_MICRO_MAX_DIMS 4

/**
 * @brief Model information structure
 */
typedef struct {
    const void *model_data;            /**< Pointer to model data */
    size_t model_size;                 /**< Size of model in bytes */
    tflite_micro_quant_type_t quantization; /**< Quantization type */
    tflite_micro_accel_type_t acceleration; /**< Hardware acceleration */
    uint32_t input_count;              /**< Number of input tensors */
    uint32_t output_count;             /**< Number of output tensors */
    size_t arena_size;                /**< Required arena size */
} tflite_micro_model_info_t;

/**
 * @brief Tensor information structure
 */
typedef struct {
    void *data;                        /**< Tensor data pointer */
    size_t size;                       /**< Data size in bytes */
    uint32_t dims_count;               /**< Number of dimensions */
    int32_t dims[4];                   /**< Dimension sizes */
    tflite_micro_quant_type_t type;    /**< Data type */
    float scale;                       /**< Quantization scale */
    int32_t zero_point;                /**< Quantization zero point */
} tflite_micro_tensor_t;

/**
 * @brief Inference context (opaque handle)
 */
typedef struct tflite_micro_context tflite_micro_context_t;

/* ========================================================================== */
/* Initialization and Management API                                          */
/* ========================================================================== */

/**
 * @brief Initialize TensorFlow Lite Micro subsystem
 * 
 * @return TFLITE_MICRO_SUCCESS on success, error code otherwise
 */
tflite_micro_result_t tflite_micro_init(void);

/**
 * @brief Clean up TensorFlow Lite Micro subsystem
 * 
 * @return TFLITE_MICRO_SUCCESS on success, error code otherwise
 */
tflite_micro_result_t tflite_micro_cleanup(void);

/**
 * @brief Check if TensorFlow Lite Micro is initialized
 * 
 * @return true if initialized, false otherwise
 */
bool tflite_micro_is_initialized(void);

/**
 * @brief Get TensorFlow Lite Micro version string
 * 
 * @return Version string
 */
const char *tflite_micro_get_version(void);

/* ========================================================================== */
/* Model Loading and Validation API                                           */
/* ========================================================================== */

/**
 * @brief Validate a TensorFlow Lite model
 * 
 * @param model_data Pointer to model data
 * @param model_size Size of model data
 * @param model_info Output model information
 * 
 * @return TFLITE_MICRO_SUCCESS on success, error code otherwise
 */
tflite_micro_result_t tflite_micro_validate_model(
    const void *model_data,
    size_t model_size,
    tflite_micro_model_info_t *model_info
);

/**
 * @brief Create inference context from model
 * 
 * @param model_data Pointer to model data
 * @param model_size Size of model data
 * @param arena_buffer Memory arena for inference
 * @param arena_size Size of memory arena
 * @param acccel_type Hardware acceleration type
 * @param context Output inference context
 * 
 * @return TFLITE_MICRO_SUCCESS on success, error code otherwise
 */
tflite_micro_result_t tflite_micro_create_context(
    const void *model_data,
    size_t model_size,
    void *arena_buffer,
    size_t arena_size,
    tflite_micro_accel_type_t accel_type,
    tflite_micro_context_t **context
);

/**
 * @brief Destroy inference context
 * 
 * @param context Inference context to destroy
 * 
 * @return TFLITE_MICRO_SUCCESS on success, error code otherwise
 */
tflite_micro_result_t tflite_micro_destroy_context(
    tflite_micro_context_t *context
);

/* ========================================================================== */
/* Tensor Access API                                                          */
/* ========================================================================== */

/**
 * @brief Get input tensor information
 * 
 * @param context Inference context
 * @param index Input tensor index
 * @param tensor Output tensor information
 * 
 * @return TFLITE_MICRO_SUCCESS on success, error code otherwise
 */
tflite_micro_result_t tflite_micro_get_input_tensor(
    tflite_micro_context_t *context,
    uint32_t index,
    tflite_micro_tensor_t *tensor
);

/**
 * @brief Get output tensor information
 * 
 * @param context Inference context
 * @param index Output tensor index  
 * @param tensor Output tensor information
 * 
 * @return TFLITE_MICRO_SUCCESS on success, error code otherwise
 */
tflite_micro_result_t tflite_micro_get_output_tensor(
    tflite_micro_context_t *context,
    uint32_t index,
    tflite_micro_tensor_t *tensor
);

/**
 * @brief Set input tensor data
 * 
 * @param context Inference context
 * @param index Input tensor index
 * @param data Input data
 * @param size Size of input data
 * 
 * @return TFLITE_MICRO_SUCCESS on success, error code otherwise
 */
tflite_micro_result_t tflite_micro_set_input_data(
    tflite_micro_context_t *context,
    uint32_t index,
    const void *data,
    size_t size
);

/**
 * @brief Get output tensor data
 * 
 * @param context Inference context
 * @param index Output tensor index
 * @param data Output buffer
 * @param size Size of output buffer
 * @param actual_size Actual size of output data
 * 
 * @return TFLITE_MICRO_SUCCESS on success, error code otherwise
 */
tflite_micro_result_t tflite_micro_get_output_data(
    tflite_micro_context_t *context,
    uint32_t index,
    void *data,
    size_t size,
    size_t *actual_size
);

/* ========================================================================== */
/* Inference API                                                              */
/* ========================================================================== */

/**
 * @brief Run inference
 * 
 * @param context Inference context
 * 
 * @return TFLITE_MICRO_SUCCESS on success, error code otherwise
 */
tflite_micro_result_t tflite_micro_invoke(tflite_micro_context_t *context);

/**
 * @brief Run inference with timeout
 * 
 * @param context Inference context
 * @param timeout_ms Timeout in milliseconds
 * 
 * @return TFLITE_MICRO_SUCCESS on success, error code otherwise
 */
tflite_micro_result_t tflite_micro_invoke_timeout(
    tflite_micro_context_t *context,
    uint32_t timeout_ms
);

/* ========================================================================== */
/* Hardware Acceleration API                                                  */
/* ========================================================================== */

/**
 * @brief Check hardware acceleration availability
 * 
 * @param accel_type Acceleration type to check
 * 
 * @return true if available, false otherwise
 */
bool tflite_micro_is_accel_available(tflite_micro_accel_type_t accel_type);

/**
 * @brief Initialize hardware acceleration
 * 
 * @param accel_type Acceleration type to initialize
 * 
 * @return TFLITE_MICRO_SUCCESS on success, error code otherwise
 */
tflite_micro_result_t tflite_micro_init_accel(tflite_micro_accel_type_t accel_type);

/**
 * @brief Deinitialize hardware acceleration
 *
 * @param accel_type Acceleration type to deinitialize
 * 
 * @return TFLITE_MICRO_SUCCESS on success, error code otherwise
 */
tflite_micro_result_t tflite_micro_deinit_accel(tflite_micro_accel_type_t accel_type);

/* ========================================================================== */
/* Performance and Debugging API                                              */
/* ========================================================================== */

#ifdef CONFIG_ML_TFLITE_MICRO_ENABLE_PROFILING

/**
 * @brief Performance statistics structure
 */
typedef struct {
    uint32_t inference_count;          /**< Number of inferences */
    uint32_t total_time_us;            /**< Total time in microseconds */ 
    uint32_t avg_time_us;              /**< Average time per inference */
    uint32_t min_time_us;              /**< Minimum inference time */
    uint32_t max_time_us;              /**< Maximum inference time */
    uint32_t memory_peak_bytes;        /**< Peak memory usage */
    uint32_t npu_utilization_percent;  /**< NPU utilization percentage */
} tflite_micro_perf_stats_t;

/**
 * @brief Get performance statistics
 * 
 * @param context Inference context
 * @param stats Output performance statistics
 * 
 * @return TFLITE_MICRO_SUCCESS on success, error code otherwise
 */
tflite_micro_result_t tflite_micro_get_perf_stats(
    tflite_micro_context_t *context,
    tflite_micro_perf_stats_t *stats
);

/**
 * @brief Reset performance statistics
 * 
 * @param context Inference context
 * 
 * @return TFLITE_MICRO_SUCCESS on success, error code otherwise
 */
tflite_micro_result_t tflite_micro_reset_perf_stats(
    tflite_micro_context_t *context
);

/**
 * @brief Enable detailed profiling
 * 
 * @param context Inference context
 * @param enable true to enable, false to disable
 * 
 * @return TFLITE_MICRO_SUCCESS on success, error code otherwise
 */
tflite_micro_result_t tflite_micro_set_profiling(
    tflite_micro_context_t *context,
    bool enable
);

#endif /* CONFIG_ML_TFLITE_MICRO_ENABLE_PROFILING */

/* ========================================================================== */
/* Utility Functions                                                          */
/* ========================================================================== */

/**
 * @brief Convert result code to string
 * 
 * @param result Result code
 * 
 * @return String representation of result code
 */
const char *tflite_micro_result_to_string(tflite_micro_result_t result);

/**
 * @brief Convert quantization type to string
 * 
 * @param type Quantization type
 * 
 * @return String representation
 */
const char *tflite_micro_quant_type_to_string(tflite_micro_quant_type_t type);

/**
 * @brief Convert acceleration type to string
 * 
 * @param type Acceleration type
 * 
 * @return String representation
 */
const char *tflite_micro_accel_type_to_string(tflite_micro_accel_type_t type);

/* ========================================================================== */
/* Memory Management Helpers                                                  */
/* ========================================================================== */

/**
 * @brief Calculate required arena size for model
 * 
 * @param model_data Pointer to model data
 * @param model_size Size of model data
 * @param arena_size Output required arena size
 * 
 * @return TFLITE_MICRO_SUCCESS on success, error code otherwise
 */
tflite_micro_result_t tflite_micro_calculate_arena_size(
    const void *model_data,
    size_t model_size,
    size_t *arena_size
);

/**
 * @brief Allocate aligned memory for tensors
 * 
 * @param size Size to allocate
 * @param alignment Alignment requirement
 * 
 * @return Pointer to allocated memory, or NULL on error
 */
void *tflite_micro_aligned_alloc(size_t size, size_t alignment);

/**
 * @brief Free aligned memory
 * 
 * @param ptr Pointer to free
 */
void tflite_micro_aligned_free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_ML_TFLITE_MICRO_INFINEON */

#endif /* TFLITE_MICRO_ZEPHYR_H */