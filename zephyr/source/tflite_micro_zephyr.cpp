/*  
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2024 Infineon Technologies AG
 */

/**
 * @file tflite_micro_zephyr.cpp
 * @brief TensorFlow Lite Micro Zephyr Integration Implementation
 * 
 * This file implements the Zephyr integration layer for TensorFlow Lite Micro,
 * providing a clean API that leverages the existing Infineon RTOS abstraction
 * infrastructure while preserving the original TensorFlow Lite Micro source.
 */

#include "tflite_micro_zephyr.h"
#include "tflite_micro_zephyr_compat.h"

#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

/* TensorFlow Lite Micro includes */
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_allocator.h"
#include "tensorflow/lite/schema/schema_generated.h"

LOG_MODULE_REGISTER(tflite_micro_zephyr, CONFIG_ML_TFLITE_LOG_LEVEL);

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
static tflite_mutex_t g_tflite_mutex;
static bool g_mutex_initialized = false;
#endif

/* Static memory arena for default configuration */
#ifdef CONFIG_ML_TFLITE_MICRO_STATIC_MEMORY
static uint8_t g_default_arena[CONFIG_ML_TFLITE_MICRO_ARENA_SIZE] 
    __aligned(sizeof(void*));
#endif

/* ========================================================================== */
/* Internal Context Structure                                                 */
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

    /* TensorFlow Lite Micro internals */
    const tflite::Model *model;
    tflite::MicroInterpreter *interpreter;
    tflite::MicroOpResolver *resolver;
    TfLiteTensor *input_tensors;
    TfLiteTensor *output_tensors;
    uint32_t input_count;
    uint32_t output_count;
};

/* ========================================================================== */
/* Internal Helper Functions                                                  */
/* ========================================================================== */

static inline void lock_tflite_micro(void)
{
#ifdef CONFIG_MULTITHREADING
    if (g_mutex_initialized) {
        tflite_mutex_lock(&g_tflite_mutex);
    }
#endif
}

static inline void unlock_tflite_micro(void)
{
#ifdef CONFIG_MULTITHREADING  
    if (g_mutex_initialized) {
        tflite_mutex_unlock(&g_tflite_mutex);
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
        if (tflite_mutex_init(&g_tflite_mutex) != 0) {
            LOG_ERR("Failed to initialize TensorFlow Lite Micro mutex");
            return TFLITE_MICRO_ERROR_GENERIC;
        }
        g_mutex_initialized = true;
    }
#endif

    lock_tflite_micro();
    
    /* Initialize hardware acceleration */
#ifdef CONFIG_ML_TFLITE_MICRO_ETHOS_U55
    if (tflite_ethos_u55_init() == 0) {
        g_hardware_accel_mask |= BIT(TFLITE_MICRO_ACCEL_ETHOS_U55);
        LOG_INF("Ethos-U55 NPU acceleration initialized");
    }
#endif

#ifdef CONFIG_ML_TFLITE_MICRO_NNLITE
    if (tflite_nnlite_init() == 0) {
        g_hardware_accel_mask |= BIT(TFLITE_MICRO_ACCEL_NNLITE);
        LOG_INF("NNLite NPU acceleration initialized");
    }
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
    
    LOG_INF("TensorFlow Lite Micro initialized successfully");
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
        tflite_mutex_destroy(&g_tflite_mutex);
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
    return "TensorFlow Lite Micro for Zephyr v1.0.0";
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
    
    /* Fill in basic model information */
    model_info->model_data = model_data;
    model_info->model_size = model_size;
    model_info->quantization = TFLITE_MICRO_QUANT_FLOAT32; /* Default */
    model_info->acceleration = TFLITE_MICRO_ACCEL_NONE;    /* Default */
    model_info->input_count = 1;    /* Default - would need full parsing */
    model_info->output_count = 1;   /* Default - would need full parsing */ 
    model_info->arena_size = CONFIG_ML_TFLITE_MICRO_ARENA_SIZE;
    
    LOG_INF("Model validation successful, size: %zu bytes", model_size);
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
    
    /* Use default arena if none provided */
    if (!arena_buffer) {
#ifdef CONFIG_ML_TFLITE_MICRO_STATIC_MEMORY
        arena_buffer = g_default_arena;
        arena_size = sizeof(g_default_arena);
#else
        return TFLITE_MICRO_ERROR_MEMORY;
#endif
    }
    
    /* Allocate context structure */
    tflite_micro_context_t *ctx = k_malloc(sizeof(tflite_micro_context_t));
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

    /* Parse TensorFlow Lite model */
    ctx->model = tflite::GetModel(model_data);
    if (ctx->model->version() != TFLITE_SCHEMA_VERSION) {
        LOG_ERR("Model version %u not supported (expected %u)",
                ctx->model->version(), TFLITE_SCHEMA_VERSION);
        k_free(ctx);
        return TFLITE_MICRO_ERROR_MODEL;
    }
    
    /* Create appropriate resolver based on configuration */
#if defined(CONFIG_ML_TFLITE_MICRO_ALL_OPS)
    static tflite::AllOpsResolver resolver;
    ctx->resolver = &resolver;
#elif defined(CONFIG_ML_TFLITE_MICRO_MICRO_OPS_ONLY)
    /* Use selective resolver for memory efficiency */
    static tflite::MicroMutableOpResolver<16> resolver;
    
    /* Add only commonly used operators */
#if defined(CONFIG_ML_TFLITE_MICRO_CONV_OPS)
    resolver.AddConv2D();
    resolver.AddDepthwiseConv2D();
#endif
#if defined(CONFIG_ML_TFLITE_MICRO_FULLY_CONNECTED_OPS)
    resolver.AddFullyConnected();
#endif
#if defined(CONFIG_ML_TFLITE_MICRO_POOLING_OPS)
    resolver.AddAveragePool2D();
    resolver.AddMaxPool2D();
#endif
#if defined(CONFIG_ML_TFLITE_MICRO_ACTIVATION_OPS)
    resolver.AddRelu();
    resolver.AddRelu6();
    resolver.AddLogistic();
#endif
    
    ctx->resolver = &resolver;
#else
    /* Default minimal resolver */
    static tflite::MicroMutableOpResolver<8> resolver;
    resolver.AddConv2D();
    resolver.AddFullyConnected();
    resolver.AddRelu();
    ctx->resolver = &resolver;
#endif
    
    /* Create interpreter */
    ctx->interpreter = new tflite::MicroInterpreter(
        ctx->model, *ctx->resolver, 
        static_cast<uint8_t*>(arena_buffer), arena_size);
    
    if (!ctx->interpreter) {
        LOG_ERR("Failed to create TensorFlow Lite Micro interpreter");
        k_free(ctx);
        return TFLITE_MICRO_ERROR_MEMORY;
    }
    
    /* Allocate tensors */
    TfLiteStatus allocate_status = ctx->interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        LOG_ERR("Failed to allocate tensors: %d", allocate_status);
        delete ctx->interpreter;
        k_free(ctx);
        return TFLITE_MICRO_ERROR_MEMORY;
    }
    
    /* Cache tensor information for fast access */
    ctx->input_count = ctx->interpreter->inputs_size();
    ctx->output_count = ctx->interpreter->outputs_size();
    
    LOG_INF("TensorFlow Lite Micro initialized: %u inputs, %u outputs", 
            ctx->input_count, ctx->output_count);
    
    ctx->is_valid = true;
    
    *context = ctx;
    
    LOG_INF("TensorFlow Lite Micro context created successfully");
    return TFLITE_MICRO_SUCCESS;
}

tflite_micro_result_t tflite_micro_destroy_context(tflite_micro_context_t *context)
{
    tflite_micro_result_t result = validate_context(context);
    if (result != TFLITE_MICRO_SUCCESS) {
        return result;
    }
    
    /* Cleanup TensorFlow Lite Micro components */
    if (context->interpreter) {
        delete context->interpreter;
        context->interpreter = nullptr;
    }
    
    /* Note: model and resolver are static objects, no cleanup needed */
    context->model = nullptr;
    context->resolver = nullptr;
    
    context->is_valid = false;
    k_free(context);
    
    LOG_INF("TensorFlow Lite Micro context destroyed");
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
    
    if (!tensor) {
        return TFLITE_MICRO_ERROR_INVALID_ARG;
    }
    
    if (index >= context->input_count) {
        return TFLITE_MICRO_ERROR_INVALID_ARG;
    }
    
    /* Get actual tensor information from TensorFlow Lite Micro interpreter */
    TfLiteTensor *input = context->interpreter->input(index);
    if (!input) {
        return TFLITE_MICRO_ERROR_GENERIC;
    }
    
    /* Fill tensor information */
    tensor->data = input->data.data;
    tensor->size = input->bytes;
    tensor->dims_count = MIN(input->dims->size, TFLITE_MICRO_MAX_DIMS);
    
    for (int i = 0; i < tensor->dims_count; i++) {
        tensor->dims[i] = input->dims->data[i];
    }
    
    /* Convert TensorFlow Lite type to our enum */
    switch (input->type) {
        case kTfLiteFloat32:
            tensor->type = TFLITE_MICRO_QUANT_FLOAT32;
            break;
        case kTfLiteInt8:
            tensor->type = TFLITE_MICRO_QUANT_INT8;
            break;
        case kTfLiteInt16:
            tensor->type = TFLITE_MICRO_QUANT_INT16;
            break;
        case kTfLiteUInt8:
            tensor->type = TFLITE_MICRO_QUANT_UINT8;
            break;
        default:
            tensor->type = TFLITE_MICRO_QUANT_UNKNOWN;
            break;
    }
    
    /* Get quantization parameters */
    if (input->quantization.type == kTfLiteAffineQuantization) {
        const TfLiteAffineQuantization* quant = 
            static_cast<const TfLiteAffineQuantization*>(input->quantization.params);
        if (quant && quant->scale && quant->zero_point) {
            tensor->scale = quant->scale->data[0];
            tensor->zero_point = quant->zero_point->data[0];
        }
    } else {
        tensor->scale = 1.0f;
        tensor->zero_point = 0;
    }
    
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
    
    if (!tensor) {
        return TFLITE_MICRO_ERROR_INVALID_ARG;
    }
    
    if (index >= context->output_count) {
        return TFLITE_MICRO_ERROR_INVALID_ARG;  
    }
    
    /* Get actual output tensor information from TensorFlow Lite Micro interpreter */
    TfLiteTensor *output = context->interpreter->output(index);
    if (!output) {
        return TFLITE_MICRO_ERROR_GENERIC;
    }
    
    /* Fill tensor information */
    tensor->data = output->data.data;
    tensor->size = output->bytes;
    tensor->dims_count = MIN(output->dims->size, TFLITE_MICRO_MAX_DIMS);
    
    for (int i = 0; i < tensor->dims_count; i++) {
        tensor->dims[i] = output->dims->data[i];
    }
    
    /* Convert TensorFlow Lite type to our enum */
    switch (output->type) {
        case kTfLiteFloat32:
            tensor->type = TFLITE_MICRO_QUANT_FLOAT32;
            break;
        case kTfLiteInt8:
            tensor->type = TFLITE_MICRO_QUANT_INT8;
            break;
        case kTfLiteInt16:
            tensor->type = TFLITE_MICRO_QUANT_INT16;
            break;
        case kTfLiteUInt8:
            tensor->type = TFLITE_MICRO_QUANT_UINT8;
            break;
        default:
            tensor->type = TFLITE_MICRO_QUANT_UNKNOWN;
            break;
    }
    
    /* Get quantization parameters */
    if (output->quantization.type == kTfLiteAffineQuantization) {
        const TfLiteAffineQuantization* quant = 
            static_cast<const TfLiteAffineQuantization*>(output->quantization.params);
        if (quant && quant->scale && quant->zero_point) {
            tensor->scale = quant->scale->data[0];
            tensor->zero_point = quant->zero_point->data[0];
        }
    } else {
        tensor->scale = 1.0f;
        tensor->zero_point = 0;
    }
    
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
    
    if (!data) {
        return TFLITE_MICRO_ERROR_INVALID_ARG;
    }
    
    if (index >= context->input_count) {
        return TFLITE_MICRO_ERROR_INVALID_ARG;
    }
    
    /* Get input tensor */
    TfLiteTensor *input = context->interpreter->input(index);
    if (!input) {
        return TFLITE_MICRO_ERROR_GENERIC;
    }
    
    /* Validate size */
    if (size > input->bytes) {
        LOG_ERR("Input data size %zu exceeds tensor size %d", size, input->bytes);
        return TFLITE_MICRO_ERROR_INVALID_ARG;
    }
    
    /* Copy input data to tensor */
    memcpy(input->data.data, data, size);
    
    /* Zero-pad remaining bytes if needed */
    if (size < input->bytes) {
        memset((uint8_t*)input->data.data + size, 0, input->bytes - size);
    }
    
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
    
    if (!data || !actual_size) {
        return TFLITE_MICRO_ERROR_INVALID_ARG;
    }
    
    if (index >= context->output_count) {
        return TFLITE_MICRO_ERROR_INVALID_ARG;
    }
    
    /* Get output tensor */
    TfLiteTensor *output = context->interpreter->output(index);
    if (!output) {
        return TFLITE_MICRO_ERROR_GENERIC;
    }
    
    /* Determine actual output size */
    size_t output_bytes = output->bytes;
    *actual_size = output_bytes;
    
    /* Check if provided buffer is large enough */
    if (size < output_bytes) {
        LOG_ERR("Output buffer size %zu too small for %zu bytes", size, output_bytes);
        return TFLITE_MICRO_ERROR_INVALID_ARG;
    }
    
    /* Copy output data from tensor */
    memcpy(data, output->data.data, output_bytes);
    
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
        start_time = tflite_get_timestamp();
    }
#endif

    /* Call TensorFlow Lite Micro inference */
    TfLiteStatus invoke_status = context->interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
        LOG_ERR("TensorFlow Lite Micro inference failed: %d", invoke_status);
#ifdef CONFIG_ML_TFLITE_MICRO_ENABLE_PROFILING
        if (context->profiling_enabled) {
            context->stats.inference_count++; /* Count failed inferences */
        }
#endif
        return TFLITE_MICRO_ERROR_INFERENCE;
    }
    
    /* Hardware acceleration dispatch - future enhancement for custom delegate */
    if (context->accel_type != TFLITE_MICRO_ACCEL_NONE) {
        LOG_DBG("Hardware acceleration %s requested (note: will be implemented via custom delegate)",
                tflite_micro_accel_type_to_string(context->accel_type));
    }

#ifdef CONFIG_ML_TFLITE_MICRO_ENABLE_PROFILING
    if (context->profiling_enabled) {
        uint32_t elapsed_us = tflite_elapsed_us(start_time);
        
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
    /* TODO: Implement timeout-based inference */
    /* For now, just call regular invoke */
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
    /* Hardware acceleration is initialized during tflite_micro_init() */
    if (tflite_micro_is_accel_available(accel_type)) {
        return TFLITE_MICRO_SUCCESS;
    }
    
    return TFLITE_MICRO_ERROR_HARDWARE;
}

tflite_micro_result_t tflite_micro_deinit_accel(tflite_micro_accel_type_t accel_type)
{
    /* Hardware acceleration cleanup is handled during tflite_micro_cleanup() */
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

tflite_micro_result_t tflite_micro_calculate_arena_size(
    const void *model_data,
    size_t model_size,
    size_t *arena_size)
{
    if (!model_data || !arena_size) {
        return TFLITE_MICRO_ERROR_INVALID_ARG;
    }
    
    /* Parse model to estimate arena requirements */
    const tflite::Model *model = tflite::GetModel(model_data);
    if (!model || model->version() != TFLITE_SCHEMA_VERSION) {
        return TFLITE_MICRO_ERROR_MODEL;
    }
    
    /* Base calculation: model size + overhead + tensor allocation */
    size_t estimated_size = model_size * 2;  /* Model data + working memory */
    estimated_size += 16384;  /* Base allocator overhead */
    estimated_size += 8192;   /* Op resolver and interpreter overhead */
    
    /* Align to next power of 2 for better memory alignment */
    size_t aligned_size = 1;
    while (aligned_size < estimated_size) {
        aligned_size <<= 1;
    }
    
    /* Ensure minimum size for any model */
    if (aligned_size < CONFIG_ML_TFLITE_MICRO_ARENA_SIZE) {
        aligned_size = CONFIG_ML_TFLITE_MICRO_ARENA_SIZE;
    }
    
    *arena_size = aligned_size;
    LOG_DBG("Calculated arena size: %zu bytes for model size: %zu bytes", 
            *arena_size, model_size);
    
    return TFLITE_MICRO_SUCCESS;
}

void *tflite_micro_aligned_alloc(size_t size, size_t alignment)
{
    /* TODO: Implement aligned allocation using Zephyr heap */
    /* For now, use regular allocation */
    ARG_UNUSED(alignment);
    return k_malloc(size);
}

void tflite_micro_aligned_free(void *ptr)
{
    k_free(ptr);
}

/* ========================================================================== */
/* Hardware Acceleration Stubs                                               */
/* ========================================================================== */

#ifdef CONFIG_ML_TFLITE_MICRO_ETHOS_U55
__weak int tflite_ethos_u55_init(void)
{
    LOG_INF("Ethos-U55 initialization (stub)");
    return 0;
}

__weak int tflite_ethos_u55_invoke(void *model_data, void *input_data, void *output_data)
{
    ARG_UNUSED(model_data);
    ARG_UNUSED(input_data);
    ARG_UNUSED(output_data);
    LOG_DBG("Ethos-U55 invoke (stub)");
    return 0;
}
#endif

#ifdef CONFIG_ML_TFLITE_MICRO_NNLITE
__weak int tflite_nnlite_init(void)
{
    LOG_INF("NNLite initialization (stub)");
    return 0;
}

__weak int tflite_nnlite_invoke(void *model_data, void *input_data, void *output_data)
{
    ARG_UNUSED(model_data);
    ARG_UNUSED(input_data);
    ARG_UNUSED(output_data);
    LOG_DBG("NNLite invoke (stub)");
    return 0;
}
#endif

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

SYS_INIT(tflite_micro_auto_init, APPLICATION, TFLITE_MICRO_AUTO_INIT_PRIORITY);

#endif /* CONFIG_ML_TFLITE_MICRO_AUTO_INIT */

} /* extern "C" */