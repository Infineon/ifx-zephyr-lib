/*
 * Copyright (c) 2026 Infineon Technologies AG,
 * or an affiliate of Infineon Technologies AG.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file cy_log_compatibility.h
 * @brief Compatibility header for cy_log from connectivity-utilities
 * 
 * This header provides compatibility between the cy_log API
 * expected by audio-front-end module and the mw-utilities cy_log
 * implementation.
 */

#ifndef CY_LOG_COMPATIBILITY_H__
#define CY_LOG_COMPATIBILITY_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Include the actual cy_log implementation */
#include "cy_log.h"

/* Ensure cy_log_init function is available */
#ifndef CY_LOG_INIT_DEFINED
#define CY_LOG_INIT_DEFINED

/* If cy_log_init is not defined in cy_log.h, provide stub */
static inline int cy_log_init(void)
{
    /* Stub implementation for compatibility */
    return 0;
}

#endif /* CY_LOG_INIT_DEFINED */

#ifdef __cplusplus
}
#endif

#endif /* CY_LOG_COMPATIBILITY_H__ */