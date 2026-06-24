/*
 * Copyright (c) 2026 Infineon Technologies AG
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file mtb_ml_zephyr.c
 * @brief Zephyr-specific implementation for Infineon ML Middleware
 *
 * This file provides Zephyr-specific initialization and utility functions
 * for the ML middleware integration layer.
 */

#include "mtb_ml_zephyr.h"
#include <zephyr/logging/log.h>
#include <zephyr/init.h>

LOG_MODULE_REGISTER(ml_middleware, CONFIG_ML_LOG_LEVEL);

/*******************************************************************************
 * Global Variables
 ******************************************************************************/

/* Global initialization flag */
static bool g_ml_middleware_initialized = false;

/*******************************************************************************
 * Static Functions
 ******************************************************************************/

#ifdef CONFIG_ML_MIDDLEWARE_ETHOS_U55
/**
 * @brief Initialize Ethos-U55 NPU if available
 */
static cy_rslt_t mtb_ml_init_ethos_u55(void)
{
    LOG_INF("Initializing ARM Ethos-U55 NPU support");

    /* NPU initialization will be handled by the original ml-middleware */
    /* This function can add any Zephyr-specific NPU setup if needed */

    return CY_RSLT_SUCCESS;
}
#endif

#if defined(CONFIG_ML_MIDDLEWARE_NNLITE) || defined(CONFIG_ML_MIDDLEWARE_ETHOS_U55)
/*
 * Zephyr-idiomatic Cy_SysInt_Init shim for the PSE84 NNLite and Ethos-U55 builds.
 *
 * cy_nn_kernel.c calls Cy_SysInt_Init() to register the NNLite and DMA ISRs.
 * mtb_ml_ethosu.c (COMPONENT_U55) does the same for the Ethos-U55 IRQ.
 *
 * The authoritative Zephyr pattern (used by cat1a/common/soc.c and
 * cat1b/cyw20829/soc.c) is to provide this function using Zephyr's
 * irq_connect_dynamic() rather than compiling cy_sysint_v2.c from the PDL.
 *
 * cy_sysint_v2.c cannot be used in a Zephyr build because it references
 * __s_vector_table_rw / __ns_vector_table_rw — symbols that exist only in
 * Infineon's PDL startup (s_start_pse84.c / ns_start_pse84.c), which Zephyr
 * replaces with its own reset-vector and does not compile.
 *
 * Using irq_connect_dynamic() is correct: it registers the handler in Zephyr's
 * vector table, sets the priority, and works for both S-world and NS builds.
 * cy_nn_kernel.c follows up with NVIC_EnableIRQ(), which is a CMSIS inline and
 * needs no additional wrapping.
 */
#include <zephyr/irq.h>
#include <cy_sysint.h>

cy_en_sysint_status_t Cy_SysInt_Init(const cy_stc_sysint_t *config,
                                      cy_israddress userIsr)
{
        cy_en_sysint_status_t status = CY_SYSINT_SUCCESS;

        /* NOTE: PendSV is assigned the lowest IRQ priority in Zephyr.
         * Cap the requested priority to IRQ_PRIO_LOWEST to avoid the
         * assertion inside z_arm_irq_priority_set(). */
#if defined(CONFIG_DYNAMIC_INTERRUPTS) && defined(CONFIG_GEN_ISR_TABLES)
        if (config != NULL) {
                uint32_t priority = (config->intrPriority > IRQ_PRIO_LOWEST)
                                  ? IRQ_PRIO_LOWEST : config->intrPriority;

                (void)irq_connect_dynamic(config->intrSrc, priority,
                                          (void *)userIsr, NULL, 0U);
        } else {
                status = CY_SYSINT_BAD_PARAM;
        }
#endif
        return status;
}
#endif /* CONFIG_ML_MIDDLEWARE_NNLITE || CONFIG_ML_MIDDLEWARE_ETHOS_U55 */

#ifdef CONFIG_ML_MIDDLEWARE_NNLITE
/**
 * @brief Initialize NNLite NPU if available
 */
static cy_rslt_t mtb_ml_init_nnlite(void)
{
    LOG_INF("Initializing Infineon NNLite NPU support");

    /* NPU initialization will be handled by the original ml-middleware */
    /* This function can add any Zephyr-specific NPU setup if needed */

    return CY_RSLT_SUCCESS;
}
#endif

/*******************************************************************************
 * Public Functions
 ******************************************************************************/

cy_rslt_t mtb_ml_zephyr_init(void)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    if (g_ml_middleware_initialized) {
        LOG_WRN("ML Middleware already initialized");
        return CY_RSLT_SUCCESS;
    }

    LOG_INF("Initializing Infineon ML Middleware for Zephyr");
    LOG_INF("NPU Type: %s", mtb_ml_get_npu_type());
    LOG_INF("Quantization: %s", mtb_ml_get_quantization_type());
    LOG_INF("Inference Engine: %s", mtb_ml_get_inference_engine());
    LOG_INF("Tensor Arena Size: %zu bytes", mtb_ml_get_default_tensor_arena_size());

#if defined(CONFIG_ML_MIDDLEWARE_ETHOS_U55)
    result = mtb_ml_init_ethos_u55();
    if (result != CY_RSLT_SUCCESS) {
        LOG_ERR("Ethos-U55 init failed (0x%08x)", (unsigned int)result);
        return result;
    }
#elif defined(CONFIG_ML_MIDDLEWARE_NNLITE)
    result = mtb_ml_init_nnlite();
    if (result != CY_RSLT_SUCCESS) {
        LOG_ERR("NNLite init failed (0x%08x)", (unsigned int)result);
        return result;
    }
#endif

    /* Do NOT call mtb_ml_init() here.
     * mtb_ml_init() is a reference-counted call — each invocation increments
     * an internal counter, and mtb_ml_deinit() decrements it.
     * The application (or DEEPCRAFT model via IMAI_init()) owns this lifecycle.
     * Calling it here would cause the state counter to reach 2 after IMAI_init(),
     * making a single mtb_ml_deinit() insufficient to fully deinitialise. */

    g_ml_middleware_initialized = true;
    LOG_INF("ML Middleware initialization completed successfully");

    return CY_RSLT_SUCCESS;
}

/*******************************************************************************
 * System Initialization
 ******************************************************************************/

/**
 * @brief System initialization hook for ML middleware
 *
 * This function is called automatically during Zephyr system initialization
 * if CONFIG_ML_MIDDLEWARE_AUTO_INIT is enabled.
 */
#ifdef CONFIG_ML_MIDDLEWARE_AUTO_INIT
static int ml_middleware_sys_init(void)
{
    cy_rslt_t result = mtb_ml_zephyr_init();
    if (result != CY_RSLT_SUCCESS) {
        LOG_ERR("System ML middleware initialization failed: 0x%08x", result);
        return -EIO;
    }
    return 0;
}

/* Register system initialization at POST_KERNEL level */
SYS_INIT(ml_middleware_sys_init, POST_KERNEL, CONFIG_ML_MIDDLEWARE_INIT_PRIORITY);
#endif /* CONFIG_ML_MIDDLEWARE_AUTO_INIT */

/*******************************************************************************
 * Utility Functions
 ******************************************************************************/

bool mtb_ml_is_initialized(void)
{
    return g_ml_middleware_initialized;
}

void mtb_ml_print_configuration(void)
{
    printk("\n=== Infineon ML Middleware Configuration ===\n");
    printk("NPU: %s\n", mtb_ml_get_npu_type());
    printk("Quantization: %s\n", mtb_ml_get_quantization_type());
    printk("Inference Engine: %s\n", mtb_ml_get_inference_engine());
    printk("Tensor Arena Size: %zu bytes\n", mtb_ml_get_default_tensor_arena_size());
    printk("Max Models: %u\n", mtb_ml_get_max_models());
    printk("NPU Available: %s\n", mtb_ml_is_npu_available() ? "Yes" : "No");

#ifdef CONFIG_ML_MIDDLEWARE_STREAMING
    printk("Streaming: Enabled\n");
#else
    printk("Streaming: Disabled\n");
#endif

#ifdef CONFIG_ML_MIDDLEWARE_PERFORMANCE_MONITORING
    printk("Performance Monitoring: Enabled\n");
#else
    printk("Performance Monitoring: Disabled\n");
#endif

#ifdef CONFIG_ML_MIDDLEWARE_CACHE_MANAGEMENT
    printk("Cache Management: Enabled\n");
#else
    printk("Cache Management: Disabled\n");
#endif

    printk("===========================================\n\n");
}