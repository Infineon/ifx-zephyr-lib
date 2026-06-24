/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2024 Infineon Technologies AG
 */

/**
 * @file tflite_micro_zephyr_compat.h
 * @brief TensorFlow Lite Micro compatibility layer for Zephyr
 * 
 * This header provides compatibility mappings between TensorFlow Lite Micro's
 * platform requirements and Zephyr RTOS APIs. It leverages the existing
 * Infineon RTOS abstraction infrastructure for consistency.
 */

#ifndef TFLITE_MICRO_ZEPHYR_COMPAT_H
#define TFLITE_MICRO_ZEPHYR_COMPAT_H

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/math_extras.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

/* Include Infineon RTOS abstraction for consistency */
#include "cyabs_rtos.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/* Platform Detection and Configuration                                      */
/* ========================================================================== */

#define TF_LITE_MCU 1
#define TF_LITE_STATIC_MEMORY 1

/* Disable features not suitable for embedded systems */
#define TF_LITE_DISABLE_X86_NEON 1
#define TF_LITE_USE_CPPSTD 0

/* ========================================================================== */
/* Memory Management Compatibility                                           */
/* ========================================================================== */

/* Use Zephyr heap for dynamic memory (when allowed) */
#ifdef CONFIG_HEAP_MEM_POOL_SIZE
#include <zephyr/sys/heap_listener.h>
#define tflite_malloc(size)    k_malloc(size)
#define tflite_free(ptr)       k_free(ptr)
#define tflite_calloc(n, size) k_calloc(n, size)
#else
/* Fall back to static allocation only */
#define tflite_malloc(size)    NULL
#define tflite_free(ptr)       
#define tflite_calloc(n, size) NULL
#endif

/* Memory alignment utilities */
#define TFLITE_ALIGN(x, a)     ROUND_UP(x, a)
#define TFLITE_IS_ALIGNED(x, a) IS_ALIGNED(x, a)

/* Cache management for NPU operations */
#if defined(CONFIG_ML_TFLITE_MICRO_ETHOS_U55) || defined(CONFIG_ML_TFLITE_MICRO_NNLITE)
#include <zephyr/cache.h>

static inline void tflite_cache_flush(void *addr, size_t size)
{
#ifdef CONFIG_DCACHE
    sys_cache_data_flush_range(addr, size);
#endif
}

static inline void tflite_cache_invalidate(void *addr, size_t size)
{
#ifdef CONFIG_DCACHE
    sys_cache_data_invd_range(addr, size);
#endif
}

#else
#define tflite_cache_flush(addr, size)     do { } while (0)
#define tflite_cache_invalidate(addr, size) do { } while (0)
#endif

/* ========================================================================== */
/* Error Reporting and Logging                                               */
/* ========================================================================== */

/* Use Zephyr logging infrastructure */
/* Note: LOG_MODULE_REGISTER is in tflite_micro_zephyr_stub.cpp */

/* TensorFlow Lite error reporting macros */
#define TF_LITE_REPORT_ERROR(...) LOG_ERR(__VA_ARGS__)

#ifdef CONFIG_ML_TFLITE_MICRO_ERROR_REPORTER_CUSTOM

/* Custom error reporter using Zephyr logging */
extern void tflite_micro_error_reporter_log(const char* format, ...);

#define TF_LITE_USE_ZEPHYR_ERROR_REPORTER 1

/* Map TensorFlow Lite debug output to Zephyr logging */
#define DebugLog(...)      LOG_DBG(__VA_ARGS__)
#define MicroPrintf(...)   LOG_INF(__VA_ARGS__)

#endif /* CONFIG_ML_TFLITE_MICRO_ERROR_REPORTER_CUSTOM */

/* ========================================================================== */
/* Timing and Profiling                                                      */
/* ========================================================================== */

#ifdef CONFIG_ML_TFLITE_MICRO_ENABLE_PROFILING

#include <zephyr/sys/time_units.h>

/* Timing utilities for profiling */
typedef uint64_t tflite_timestamp_t;

static inline tflite_timestamp_t tflite_get_timestamp(void)
{
    return k_cycle_get_64();
}

static inline uint32_t tflite_get_time_ms(void)
{
    return k_uptime_get_32();
}

static inline uint32_t tflite_get_time_us(void)
{
    return k_cyc_to_us_floor64(k_cycle_get_64());
}

static inline uint32_t tflite_elapsed_us(tflite_timestamp_t start)
{
    return k_cyc_to_us_floor64(k_cycle_get_64() - start);
}

/* Profiling macros */
#define TFLITE_MICRO_PROFILE_START() tflite_get_timestamp()
#define TFLITE_MICRO_PROFILE_END(start) tflite_elapsed_us(start)

#endif /* CONFIG_ML_TFLITE_MICRO_ENABLE_PROFILING */

/* ========================================================================== */
/* Thread Safety and Synchronization                                         */
/* ========================================================================== */

/* Use RTOS abstraction for thread safety */
#ifdef CONFIG_MULTITHREADING

typedef cy_mutex_t tflite_mutex_t;

static inline int tflite_mutex_init(tflite_mutex_t *mutex)
{
    cy_rslt_t result = cy_rtos_init_mutex(mutex);
    return (result == CY_RSLT_SUCCESS) ? 0 : -1;
}

static inline void tflite_mutex_lock(tflite_mutex_t *mutex)
{
    cy_rtos_get_mutex(mutex, CY_RTOS_NEVER_TIMEOUT);
}

static inline void tflite_mutex_unlock(tflite_mutex_t *mutex)
{
    cy_rtos_set_mutex(mutex);
}

static inline void tflite_mutex_destroy(tflite_mutex_t *mutex)
{
    cy_rtos_deinit_mutex(mutex);
}

#else
/* Single-threaded fallback */
typedef int tflite_mutex_t;
#define tflite_mutex_init(m)    0
#define tflite_mutex_lock(m)    do { } while (0)
#define tflite_mutex_unlock(m)  do { } while (0)
#define tflite_mutex_destroy(m) do { } while (0)
#endif

/* ========================================================================== */
/* Mathematical Functions                                                     */
/* ========================================================================== */

#include <math.h>

/* Use Zephyr DSP library if available */
#ifdef CONFIG_DSP
#include <zephyr/dsp/dsp.h>
#include <zephyr/dsp/types.h>
#include <zephyr/dsp/print_format.h>
#endif

/* Fast approximations for embedded use */
#ifndef CONFIG_FPU
/* Integer-based approximations when FPU is not available */
extern float tflite_fast_exp(float x);
extern float tflite_fast_tanh(float x);
extern float tflite_fast_sigmoid(float x);

#define expf(x)   tflite_fast_exp(x)
#define tanhf(x)  tflite_fast_tanh(x)
#endif

/* ========================================================================== */
/* Hardware Acceleration Support                                             */
/* ========================================================================== */

#ifdef CONFIG_ML_TFLITE_MICRO_ETHOS_U55
/* ARM Ethos-U55 NPU support */
#define ETHOS_U55_SUPPORT 1
extern int tflite_ethos_u55_init(void);
extern int tflite_ethos_u55_invoke(void *model_data, void *input_data, void *output_data);
#endif

#ifdef CONFIG_ML_TFLITE_MICRO_NNLITE
/* Infineon NNLite NPU support */
#define NNLITE_SUPPORT 1
extern int tflite_nnlite_init(void);
extern int tflite_nnlite_invoke(void *model_data, void *input_data, void *output_data);
#endif

#ifdef CONFIG_ML_TFLITE_MICRO_CMSIS_NN
/* ARM CMSIS-NN optimization */
#define CMSIS_NN 1
#include <arm_nnsupportfunctions.h>
#include <arm_nnfunctions.h>
#endif

/* ========================================================================== */
/* Platform-specific Optimizations                                           */
/* ========================================================================== */

/* Memory copy optimizations */
#ifdef CONFIG_ARCH_HAS_CUSTOM_MEMCPY_DMA
extern void *tflite_memcpy_dma(void *dest, const void *src, size_t n);
#define tflite_memcpy tflite_memcpy_dma
#else
#define tflite_memcpy memcpy
#endif

/* Unaligned access support */
#ifdef CONFIG_UNALIGNED_ACCESS
#define TFLITE_ALLOW_UNALIGNED_ACCESS 1
#else
#define TFLITE_ALLOW_UNALIGNED_ACCESS 0
#endif

/* ========================================================================== */
/* Debugging and Development Support                                          */
/* ========================================================================== */

#ifdef CONFIG_ML_TFLITE_MICRO_BENCHMARK_MODE
/* Benchmark mode utilities */
extern void tflite_benchmark_start(const char *operation);
extern void tflite_benchmark_end(const char *operation);
extern void tflite_benchmark_print_results(void);

#define TFLITE_BENCHMARK_START(op) tflite_benchmark_start(op)
#define TFLITE_BENCHMARK_END(op)   tflite_benchmark_end(op)
#else
#define TFLITE_BENCHMARK_START(op) 
#define TFLITE_BENCHMARK_END(op)   
#endif

/* Assert implementation */
#ifdef CONFIG_ASSERT
#include <zephyr/sys/__assert.h>
#define TFLITE_MICRO_ASSERT(condition) __ASSERT(condition, "TensorFlow Lite Micro assertion failed")
#else
#define TFLITE_MICRO_ASSERT(condition) do { } while (0)
#endif

/* ========================================================================== */
/* Static Analysis and Bounds Checking                                       */
/* ========================================================================== */

/* Array bounds checking in debug mode */
#ifdef CONFIG_DEBUG
#define TFLITE_ARRAY_BOUNDS_CHECK(array, index, size) \
    TFLITE_MICRO_ASSERT((index) < (size))
#else
#define TFLITE_ARRAY_BOUNDS_CHECK(array, index, size) do { } while (0)
#endif

/* ========================================================================== */
/* Initialization Support                                                     */
/* ========================================================================== */

/* Early initialization function */
extern int tflite_micro_zephyr_early_init(void);

/* Standard initialization function */
extern int tflite_micro_zephyr_init(void);

/* Cleanup function */
extern void tflite_micro_zephyr_cleanup(void);

/* Auto-initialization support */
#ifdef CONFIG_ML_TFLITE_MICRO_AUTO_INIT
#define TFLITE_MICRO_AUTO_INIT_PRIORITY CONFIG_ML_TFLITE_MICRO_INIT_PRIORITY
#endif

#ifdef __cplusplus
}
#endif

#endif /* TFLITE_MICRO_ZEPHYR_COMPAT_H */