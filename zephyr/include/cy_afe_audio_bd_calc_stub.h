/*
 * Copyright (c) 2026 Infineon Technologies AG
 * SPDX-License-Identifier: Apache-2.0
 *
 * AFE Bulk Delay Calculation stub declarations for basic mode
 */

#ifndef CY_AFE_AUDIO_BD_CALC_STUB_H__
#define CY_AFE_AUDIO_BD_CALC_STUB_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CONFIG_AUDIOFE_SPEECH_ENH

#include "cy_result.h"

/* Forward declarations for types used by bd_calc */
typedef void afe_internal_context_t;
typedef struct {
    int16_t *input1;
    int16_t *input2;
    int16_t *aec_reference_input;
    int16_t *output;
    int16_t *ifx_internal_output;
} afe_sp_enh_input_output_t;

/* Function declarations */
cy_rslt_t cy_afe_bd_calc_process(afe_internal_context_t *context,
        afe_sp_enh_input_output_t *sp_enh_input_output);
cy_rslt_t cy_afe_bd_calc_wait_for_complete(unsigned int ui_timeout_ms,
        int *bulk_delay);

#endif /* !CONFIG_AUDIOFE_SPEECH_ENH */

#ifdef __cplusplus
}
#endif

#endif /* CY_AFE_AUDIO_BD_CALC_STUB_H__ */
