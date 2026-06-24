/*  
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2024 Infineon Technologies AG
 */

/**
 * @file tflite_micro_zephyr_stub.cpp
 * @brief TensorFlow Lite Micro Zephyr Stub Implementation for Initial Testing
 * 
 * This is a simplified stub implementation for testing NPU initialization
 * and basic inference flow without full TensorFlow Lite dependencies.
 */

#include "tflite_micro_zephyr.h"

#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <string.h>

LOG_MODULE_REGISTER(tflite_micro_zephyr_stub, LOG_LEVEL_INF);

extern "C" {

/* ========================================================================== */
/* Internal State and Configuration                                           */
/* ========================================================================== */

/* Global initialization state */
static bool g_tflite_micro_initialized = false;

/* Hardware acceleration state */
static uint32_t g_hardware_accel_mask = 0;

/* Performance tracking */
#ifdef CONFIG_ML_TFLITE_MICRO_ENABLE_PROFILING
static tflite_micro_perf_stats_t g_global_stats = {0};
#endif

/* Thread safety */
#ifdef CONFIG_MULTITHREADING
static struct k_mutex g_tflite_mutex;
static bool g_mutex_initialized = false;
#endif

/* ========================================================================== */
/* Internal Context Structure (Stub)                                         */
/* ========================================================================== */

struct tflite_micro_context {
    const void *model_data;
    size_t model_size;
    void *arena_buffer;
    size_t arena_size;
    tflite_micro_accel_type_t accel_type;
    bool is_valid;
    
    /* Performance tracking */
#ifdef CONFIG_ML_TFLITE_MICRO_ENABLE_PROFILING
    tflite_micro_perf_stats_t stats;
    uint64_t last_invoke_start;
    bool profiling_enabled;
#endif

    /* Stub model info */
    uint32_t input_count;
    uint32_t output_count;
    tflite_micro_tensor_t stub_input_tensor;
    tflite_micro_tensor_t stub_output_tensor;
};

/* ========================================================================== */
/* Internal Helper Functions                                                  */
/* ========================================================================== */

static inline void lock_tflite_micro(void)
{
#ifdef CONFIG_MULTITHREADING
    if (g_mutex_initialized) {
        k_mutex_lock(&g_tflite_mutex, K_FOREVER);
    }
#endif
}

static inline void unlock_tflite_micro(void)
{
#ifdef CONFIG_MULTITHREADING
    if (g_mutex_initialized) {
        k_mutex_unlock(&g_tflite_mutex);
    }
#endif
}

static tflite_micro_result_t validate_context(tflite_micro_context_t *context)
{
    if (!context) {
        return TFLITE_MICRO_ERROR_INVALID_ARG;
    }
    
    if (!context->is_valid) {
        return TFLITE_MICRO_ERROR_NOT_INIT;
    }
    
    return TFLITE_MICRO_SUCCESS;
}

/* ========================================================================== */
/* Initialization and Management API Implementation                           */
/* ========================================================================== */

tflite_micro_result_t tflite_micro_init(void)
{
    if (g_tflite_micro_initialized) {
        return TFLITE_MICRO_SUCCESS;
    }
    
#ifdef CONFIG_MULTITHREADING
    if (!g_mutex_initialized) {
        if (k_mutex_init(&g_tflite_mutex) != 0) {
            LOG_ERR("Failed to initialize TensorFlow Lite Micro mutex");
            return TFLITE_MICRO_ERROR_GENERIC;
        }
        g_mutex_initialized = true;
    }
#endif

    lock_tflite_micro();
    
    /* Initialize hardware acceleration (stub) */
#ifdef CONFIG_ML_TFLITE_MICRO_ETHOS_U55
    g_hardware_accel_mask |= BIT(TFLITE_MICRO_ACCEL_ETHOS_U55);
    LOG_INF("Ethos-U55 NPU stub initialized");
#endif

#ifdef CONFIG_ML_TFLITE_MICRO_NNLITE
    g_hardware_accel_mask |= BIT(TFLITE_MICRO_ACCEL_NNLITE);
    LOG_INF("NNLite NPU stub initialized");
#endif

#ifdef CONFIG_ML_TFLITE_MICRO_CMSIS_NN
    g_hardware_accel_mask |= BIT(TFLITE_MICRO_ACCEL_CMSIS_NN);
    LOG_INF("CMSIS-NN optimization enabled");
#endif

    /* Initialize performance tracking */
#ifdef CONFIG_ML_TFLITE_MICRO_ENABLE_PROFILING
    memset(&g_global_stats, 0, sizeof(g_global_stats));
    LOG_INF("TensorFlow Lite Micro profiling enabled");
#endif

    g_tflite_micro_initialized = true;
    unlock_tflite_micro();
    
    LOG_INF("TensorFlow Lite Micro stub initialized successfully");
    return TFLITE_MICRO_SUCCESS;
}

tflite_micro_result_t tflite_micro_cleanup(void)
{
    if (!g_tflite_micro_initialized) {
        return TFLITE_MICRO_SUCCESS;
    }
    
    lock_tflite_micro();
    
    /* Cleanup hardware acceleration */
    g_hardware_accel_mask = 0;
    
    g_tflite_micro_initialized = false;
    unlock_tflite_micro();

#ifdef CONFIG_MULTITHREADING
    if (g_mutex_initialized) {
        g_mutex_initialized = false;
    }
#endif
    
    LOG_INF("TensorFlow Lite Micro cleanup complete");
    return TFLITE_MICRO_SUCCESS;
}

bool tflite_micro_is_initialized(void)
{
    return g_tflite_micro_initialized;
}

const char *tflite_micro_get_version(void)
{
    return "TensorFlow Lite Micro for Zephyr v1.0.0-stub";
}

/* ========================================================================== */
/* Model Loading and Validation API Implementation                            */
/* ========================================================================== */

tflite_micro_result_t tflite_micro_validate_model(
    const void *model_data,
    size_t model_size,
    tflite_micro_model_info_t *model_info)
{
    if (!model_data || !model_info) {
        return TFLITE_MICRO_ERROR_INVALID_ARG;
    }
    
    if (!g_tflite_micro_initialized) {
        return TFLITE_MICRO_ERROR_NOT_INIT;
    }
    
    /* Basic validation - check for TensorFlow Lite magic number */
    if (model_size < 8) {
        return TFLITE_MICRO_ERROR_MODEL;
    }
    
    /* TensorFlow Lite flatbuffer magic: "TFL3" */
    const uint8_t *data = (const uint8_t *)model_data;
    if (data[4] != 'T' || data[5] != 'F' || data[6] != 'L' || data[7] != '3') {
        LOG_ERR("Invalid TensorFlow Lite model magic number");
        return TFLITE_MICRO_ERROR_MODEL;
    }
    
    /* Fill in stub model information */
    model_info->model_data = model_data;
    model_info->model_size = model_size;
    model_info->quantization = TFLITE_MICRO_QUANT_FLOAT32; 
    model_info->acceleration = TFLITE_MICRO_ACCEL_NNLITE;    
    model_info->input_count = 1;    
    model_info->output_count = 1;   
    model_info->arena_size = CONFIG_ML_TFLITE_MICRO_ARENA_SIZE;
    
    LOG_INF("Model validation successful (stub), size: %zu bytes", model_size);
    return TFLITE_MICRO_SUCCESS;
}

tflite_micro_result_t tflite_micro_create_context(
    const void *model_data,
    size_t model_size,
    void *arena_buffer,
    size_t arena_size,
    tflite_micro_accel_type_t accel_type,
    tflite_micro_context_t **context)
{
    if (!model_data || !context) {
        return TFLITE_MICRO_ERROR_INVALID_ARG;
    }
    
    if (!g_tflite_micro_initialized) {
        return TFLITE_MICRO_ERROR_NOT_INIT;
    }
    
    /* Allocate context structure */
    tflite_micro_context_t *ctx = (tflite_micro_context_t *)k_malloc(sizeof(tflite_micro_context_t));
    if (!ctx) {
        return TFLITE_MICRO_ERROR_MEMORY;
    }
    
    memset(ctx, 0, sizeof(tflite_micro_context_t));
    
    /* Initialize context */
    ctx->model_data = model_data;
    ctx->model_size = model_size;
    ctx->arena_buffer = arena_buffer;
    ctx->arena_size = arena_size;
    ctx->accel_type = accel_type;
    
#ifdef CONFIG_ML_TFLITE_MICRO_ENABLE_PROFILING
    memset(&ctx->stats, 0, sizeof(ctx->stats));
    ctx->profiling_enabled = true;
    ctx->stats.min_time_us = UINT32_MAX;
#endif

    /* Set up stub tensors */
    ctx->input_count = 1;
    ctx->output_count = 1;
    
    /* Configure stub input tensor */
    ctx->stub_input_tensor.data = NULL; /* Will be set when input data is provided */
    ctx->stub_input_tensor.size = 64;   /* 16 float32 values */
    ctx->stub_input_tensor.dims_count = 2;
    ctx->stub_input_tensor.dims[0] = 1;  /* Batch size */
    ctx->stub_input_tensor.dims[1] = 16; /* Input features */
    ctx->stub_input_tensor.type = TFLITE_MICRO_QUANT_FLOAT32;
    ctx->stub_input_tensor.scale = 1.0f;
    ctx->stub_input_tensor.zero_point = 0;
    
    /* Configure stub output tensor */
    ctx->stub_output_tensor.data = k_malloc(16); /* 4 float32 values */
    ctx->stub_output_tensor.size = 16;
    ctx->stub_output_tensor.dims_count = 2;
    ctx->stub_output_tensor.dims[0] = 1; /* Batch size */
    ctx->stub_output_tensor.dims[1] = 4; /* Output classes */
    ctx->stub_output_tensor.type = TFLITE_MICRO_QUANT_FLOAT32;
    ctx->stub_output_tensor.scale = 1.0f;
    ctx->stub_output_tensor.zero_point = 0;
    
    if (!ctx->stub_output_tensor.data) {
        k_free(ctx);
        return TFLITE_MICRO_ERROR_MEMORY;
    }
    
    // Initialize output with stub values
    float *output_data = (float *)ctx->stub_output_tensor.data;
    output_data[0] = 0.25f;
    output_data[1] = 0.35f;
    output_data[2] = 0.15f;
    output_data[3] = 0.25f;
    
    ctx->is_valid = true;
    *context = ctx;
    
    LOG_INF("TensorFlow Lite Micro stub context created with %s acceleration",
            tflite_micro_accel_type_to_string(accel_type));
    return TFLITE_MICRO_SUCCESS;
}

tflite_micro_result_t tflite_micro_destroy_context(tflite_micro_context_t *context)
{
    tflite_micro_result_t result = validate_context(context);
    if (result != TFLITE_MICRO_SUCCESS) {
        return result;
    }
    
    /* Free output tensor data */
    if (context->stub_output_tensor.data) {
        k_free(context->stub_output_tensor.data);
    }
    
    context->is_valid = false;
    k_free(context);
    
    LOG_INF("TensorFlow Lite Micro stub context destroyed");
    return TFLITE_MICRO_SUCCESS;
}

/* ========================================================================== */
/* Tensor Access API Implementation                                           */
/* ========================================================================== */

tflite_micro_result_t tflite_micro_get_input_tensor(
    tflite_micro_context_t *context,
    uint32_t index,
    tflite_micro_tensor_t *tensor)
{
    tflite_micro_result_t result = validate_context(context);
    if (result != TFLITE_MICRO_SUCCESS) {
        return result;
    }
    
    if (!tensor || index >= context->input_count) {
        return TFLITE_MICRO_ERROR_INVALID_ARG;
    }
    
    /* Return stub input tensor info */
    *tensor = context->stub_input_tensor;
    
    return TFLITE_MICRO_SUCCESS;
}

tflite_micro_result_t tflite_micro_get_output_tensor(
    tflite_micro_context_t *context,
    uint32_t index,
    tflite_micro_tensor_t *tensor)
{
    tflite_micro_result_t result = validate_context(context);
    if (result != TFLITE_MICRO_SUCCESS) {
        return result;
    }
    
    if (!tensor || index >= context->output_count) {
        return TFLITE_MICRO_ERROR_INVALID_ARG;  
    }
    
    /* Return stub output tensor info */
    *tensor = context->stub_output_tensor;
    
    return TFLITE_MICRO_SUCCESS;
}

tflite_micro_result_t tflite_micro_set_input_data(
    tflite_micro_context_t *context,
    uint32_t index,
    const void *data,
    size_t size)
{
    tflite_micro_result_t result = validate_context(context);
    if (result != TFLITE_MICRO_SUCCESS) {
        return result;
    }
    
    if (!data || index >= context->input_count) {
        return TFLITE_MICRO_ERROR_INVALID_ARG;
    }
    
    /* Store input data reference (stub) */
    context->stub_input_tensor.data = (void *)data;
    context->stub_input_tensor.size = size;
    
    LOG_DBG("Input data set: %zu bytes", size);
    return TFLITE_MICRO_SUCCESS;
}

tflite_micro_result_t tflite_micro_get_output_data(
    tflite_micro_context_t *context,
    uint32_t index,
    void *data,
    size_t size,
    size_t *actual_size)
{
    tflite_micro_result_t result = validate_context(context);
    if (result != TFLITE_MICRO_SUCCESS) {
        return result;
    }
    
    if (!data || !actual_size || index >= context->output_count) {
        return TFLITE_MICRO_ERROR_INVALID_ARG;
    }
    
    /* Copy stub output data */
    size_t output_bytes = context->stub_output_tensor.size;
    *actual_size = output_bytes;
    
    if (size < output_bytes) {
        LOG_ERR("Output buffer size %zu too small for %zu bytes", size, output_bytes);
        return TFLITE_MICRO_ERROR_INVALID_ARG;
    }
    
    memcpy(data, context->stub_output_tensor.data, output_bytes);
    
    return TFLITE_MICRO_SUCCESS;
}

/* ========================================================================== */
/* Inference API Implementation                                               */
/* ========================================================================== */

tflite_micro_result_t tflite_micro_invoke(tflite_micro_context_t *context)
{
    tflite_micro_result_t result = validate_context(context);
    if (result != TFLITE_MICRO_SUCCESS) {
        return result;
    }

#ifdef CONFIG_ML_TFLITE_MICRO_ENABLE_PROFILING
    uint64_t start_time = 0;
    if (context->profiling_enabled) {
        start_time = k_uptime_get();
    }
#endif

    /* Simulate NPU inference with processing delay */
    if (context->accel_type != TFLITE_MICRO_ACCEL_NONE) {
        LOG_DBG("NPU inference simulation (%s)",
                tflite_micro_accel_type_to_string(context->accel_type));
        
        /* Simulate NPU processing time */
        k_busy_wait(100); /* 100 microseconds */
    }
    
    /* Stub inference: modify output based on input */
    if (context->stub_input_tensor.data && context->stub_output_tensor.data) {
        float *input = (float *)context->stub_input_tensor.data;
        float *output = (float *)context->stub_output_tensor.data;
        
        /* Simple classification stub: sum inputs and distribute across outputs */
        float input_sum = 0.0f;
        for (int i = 0; i < 16 && i * sizeof(float) < context->stub_input_tensor.size; i++) {
            input_sum += input[i];
        }
        
        /* Normalize and distribute to 4 output classes */
        float scale = (input_sum > 0) ? (1.0f / (1.0f + input_sum)) : 0.5f;
        output[0] = 0.1f + scale * 0.3f;
        output[1] = 0.2f + scale * 0.4f;
        output[2] = 0.3f + scale * 0.2f;
        output[3] = 0.4f + scale * 0.1f;
        
        /* Normalize to sum to 1.0 */
        float total = output[0] + output[1] + output[2] + output[3];
        if (total > 0) {
            output[0] /= total;
            output[1] /= total;
            output[2] /= total;
            output[3] /= total;
        }
    }

#ifdef CONFIG_ML_TFLITE_MICRO_ENABLE_PROFILING
    if (context->profiling_enabled) {
        uint32_t elapsed_us = (uint32_t)((k_uptime_get() - start_time) * 1000);
        
        context->stats.inference_count++;
        context->stats.total_time_us += elapsed_us;
        context->stats.avg_time_us = context->stats.total_time_us / 
                                    context->stats.inference_count;
        
        if (elapsed_us < context->stats.min_time_us) {
            context->stats.min_time_us = elapsed_us;
        }
        
        if (elapsed_us > context->stats.max_time_us) {
            context->stats.max_time_us = elapsed_us;
        }
        
        /* Simulate NPU utilization */
        if (context->accel_type != TFLITE_MICRO_ACCEL_NONE) {
            context->stats.npu_utilization_percent = 85; /* Stub value */
        }
        
        /* Update global stats */
        g_global_stats.inference_count++;
        g_global_stats.total_time_us += elapsed_us;
        g_global_stats.avg_time_us = g_global_stats.total_time_us / 
                                    g_global_stats.inference_count;
    }
#endif

    return TFLITE_MICRO_SUCCESS;
}

tflite_micro_result_t tflite_micro_invoke_timeout(
    tflite_micro_context_t *context,
    uint32_t timeout_ms)
{
    /* For stub implementation, timeout is not needed */
    ARG_UNUSED(timeout_ms);
    return tflite_micro_invoke(context);
}

/* ========================================================================== */
/* Hardware Acceleration API Implementation                                   */
/* ========================================================================== */

bool tflite_micro_is_accel_available(tflite_micro_accel_type_t accel_type)
{
    if (accel_type >= 32) {
        return false;
    }
    
    return (g_hardware_accel_mask & BIT(accel_type)) != 0;
}

tflite_micro_result_t tflite_micro_init_accel(tflite_micro_accel_type_t accel_type)
{
    if (tflite_micro_is_accel_available(accel_type)) {
        return TFLITE_MICRO_SUCCESS;
    }
    
    return TFLITE_MICRO_ERROR_HARDWARE;
}

tflite_micro_result_t tflite_micro_deinit_accel(tflite_micro_accel_type_t accel_type)
{
    ARG_UNUSED(accel_type);
    return TFLITE_MICRO_SUCCESS;
}

/* ========================================================================== */
/* Performance and Debugging API Implementation                               */
/* ========================================================================== */

#ifdef CONFIG_ML_TFLITE_MICRO_ENABLE_PROFILING

tflite_micro_result_t tflite_micro_get_perf_stats(
    tflite_micro_context_t *context,
    tflite_micro_perf_stats_t *stats)
{
    tflite_micro_result_t result = validate_context(context);
    if (result != TFLITE_MICRO_SUCCESS) {
        return result;
    }
    
    if (!stats) {
        return TFLITE_MICRO_ERROR_INVALID_ARG;
    }
    
    memcpy(stats, &context->stats, sizeof(tflite_micro_perf_stats_t));
    return TFLITE_MICRO_SUCCESS;
}

tflite_micro_result_t tflite_micro_reset_perf_stats(
    tflite_micro_context_t *context)
{
    tflite_micro_result_t result = validate_context(context);
    if (result != TFLITE_MICRO_SUCCESS) {
        return result;
    }
    
    memset(&context->stats, 0, sizeof(tflite_micro_perf_stats_t));
    context->stats.min_time_us = UINT32_MAX;
    
    return TFLITE_MICRO_SUCCESS;
}

tflite_micro_result_t tflite_micro_set_profiling(
    tflite_micro_context_t *context,
    bool enable)
{
    tflite_micro_result_t result = validate_context(context);
    if (result != TFLITE_MICRO_SUCCESS) {
        return result;
    }
    
    context->profiling_enabled = enable;
    return TFLITE_MICRO_SUCCESS;
}

#endif /* CONFIG_ML_TFLITE_MICRO_ENABLE_PROFILING */

/* ========================================================================== */
/* Utility Functions Implementation                                           */
/* ========================================================================== */

const char *tflite_micro_result_to_string(tflite_micro_result_t result)
{
    switch (result) {
        case TFLITE_MICRO_SUCCESS:        return "Success";
        case TFLITE_MICRO_ERROR_GENERIC:  return "Generic error";
        case TFLITE_MICRO_ERROR_MEMORY:   return "Memory error";
        case TFLITE_MICRO_ERROR_INVALID_ARG: return "Invalid argument";
        case TFLITE_MICRO_ERROR_NOT_INIT: return "Not initialized";
        case TFLITE_MICRO_ERROR_HARDWARE: return "Hardware error";
        case TFLITE_MICRO_ERROR_MODEL:    return "Model error";
        case TFLITE_MICRO_ERROR_INFERENCE: return "Inference error";
        default: return "Unknown error";
    }
}

const char *tflite_micro_quant_type_to_string(tflite_micro_quant_type_t type)
{
    switch (type) {
        case TFLITE_MICRO_QUANT_NONE:    return "None";
        case TFLITE_MICRO_QUANT_INT8:    return "INT8";
        case TFLITE_MICRO_QUANT_INT16:   return "INT16";
        case TFLITE_MICRO_QUANT_UINT8:   return "UINT8";
        case TFLITE_MICRO_QUANT_FLOAT32: return "Float32";
        case TFLITE_MICRO_QUANT_UNKNOWN: return "Unknown";
        default: return "Invalid";
    }
}

const char *tflite_micro_accel_type_to_string(tflite_micro_accel_type_t type)
{
    switch (type) {
        case TFLITE_MICRO_ACCEL_NONE:     return "None";
        case TFLITE_MICRO_ACCEL_ETHOS_U55: return "Ethos-U55";
        case TFLITE_MICRO_ACCEL_NNLITE:   return "NNLite";
        case TFLITE_MICRO_ACCEL_CMSIS_NN: return "CMSIS-NN";
        default: return "Unknown";
    }
}

/* ========================================================================== */
/* Auto-Initialization Support                                                */
/* ========================================================================== */

#ifdef CONFIG_ML_TFLITE_MICRO_AUTO_INIT

static int tflite_micro_auto_init(void)
{
    tflite_micro_result_t result = tflite_micro_init();
    if (result != TFLITE_MICRO_SUCCESS) {
        LOG_ERR("Auto-initialization failed: %s", 
                tflite_micro_result_to_string(result));
        return -1;
    }
    
    return 0;
}

SYS_INIT(tflite_micro_auto_init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);

#endif /* CONFIG_ML_TFLITE_MICRO_AUTO_INIT */

} /* extern "C" */