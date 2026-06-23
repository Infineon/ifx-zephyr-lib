/*
 * Copyright (c) 2026 Infineon Technologies AG
 * SPDX-License-Identifier: Apache-2.0
 *
 * Speech enhancement compatibility wrapper for AFE.
 * Routes to real headers when CONFIG_AUDIOFE_SPEECH_ENH is enabled,
 * or to stubs when disabled.
 */

#ifndef CY_AFE_AUDIO_SPEECH_ENH_COMPAT_H__
#define CY_AFE_AUDIO_SPEECH_ENH_COMPAT_H__

#ifdef __cplusplus
extern "C" {
#endif

/* If speech enhancement is disabled, provide stub definitions for compilation */
#ifndef CONFIG_AUDIOFE_SPEECH_ENH

#include "cy_result.h"
#include <stdbool.h>
#include "ifx_sp_enh_stub.h"

/* Minimal type definitions stub to allow compilation */
typedef struct {
    CY_AFE_DATA_T *input1;
    CY_AFE_DATA_T *input2;
    CY_AFE_DATA_T *aec_reference_input;
    CY_AFE_DATA_T *output;
    CY_AFE_DATA_T *ifx_internal_output;
} afe_sp_enh_input_output_t;

typedef struct {
    void* data_point_container;
    ifx_stc_sp_enh_info_t sp_enh_info;
    char* persistent_memory;
    char* scratch_memory;
} ifx_sp_enh_context_t;

#else

/* Real speech enhancement header when feature is enabled */
#include "ifx_sp_enh.h"
#include "cy_result.h"
#include <stdbool.h>
#include "cy_afe_audio_internal.h"
#include "cyabs_rtos_impl.h"
#include "cyabs_rtos.h"
#include "cy_audio_front_end.h"

typedef struct {
    CY_AFE_DATA_T *input1;
    CY_AFE_DATA_T *input2;
    CY_AFE_DATA_T *aec_reference_input;
    CY_AFE_DATA_T *output;
    CY_AFE_DATA_T *ifx_internal_output;
} afe_sp_enh_input_output_t;

typedef struct {
    void* data_point_container;
    ifx_stc_sp_enh_info_t sp_enh_info;
    char* persistent_memory;
    char* scratch_memory;
} ifx_sp_enh_context_t;

#endif /* CONFIG_AUDIOFE_SPEECH_ENH */

#ifdef __cplusplus
}
#endif

#endif /* CY_AFE_AUDIO_SPEECH_ENH_COMPAT_H__ */
