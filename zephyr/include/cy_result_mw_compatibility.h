/*
 * Copyright (c) 2026 Infineon Technologies AG,
 * or an affiliate of Infineon Technologies AG.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file cy_result_mw_compatibility.h
 * @brief Middleware utilities cy_result_mw.h compatibility forwarding header
 * 
 * This header ensures cy_result_mw.h can find the cy_result.h definitions
 * from the audio-front-end module's compatibility layer.
 */

#ifndef CY_RESULT_MW_COMPATIBILITY_H__
#define CY_RESULT_MW_COMPATIBILITY_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Ensure cy_result.h compatibility layer is included first */
#include "cy_result.h"

/* Now include the actual cy_result_mw.h */
#include "../cy_result_mw.h"

#ifdef __cplusplus
}
#endif

#endif /* CY_RESULT_MW_COMPATIBILITY_H__ */