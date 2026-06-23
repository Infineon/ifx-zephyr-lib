/*
 * Copyright (c) 2026 Infineon Technologies AG
 * SPDX-License-Identifier: Apache-2.0
 *
 * AFE Bulk Delay Calculation Stub
 * Provides minimal implementations for bd_calc functions in basic mode.
 */

#include "cy_result.h"
#include <stdint.h>

#ifndef CONFIG_AUDIOFE_SPEECH_ENH

/* Forward declare types used by bd_calc */
typedef void afe_internal_context_t;
typedef struct {
    int16_t *input1;
    int16_t *input2;
    int16_t *aec_reference_input;
    int16_t *output;
    int16_t *ifx_internal_output;
} afe_sp_enh_input_output_t;

/* Stub: bulk delay calculation - does nothing in basic mode */
cy_rslt_t cy_afe_bd_calc_process(afe_internal_context_t *context,
        afe_sp_enh_input_output_t *sp_enh_input_output)
{
    (void)context;
    (void)sp_enh_input_output;
    return CY_RSLT_SUCCESS;
}

/* Stub: wait for bulk delay calculation complete - returns immediately */
cy_rslt_t cy_afe_bd_calc_wait_for_complete(unsigned int ui_timeout_ms,
        int *bulk_delay)
{
    (void)ui_timeout_ms;
    if (bulk_delay) *bulk_delay = 0;
    return CY_RSLT_SUCCESS;
}

#endif /* !CONFIG_AUDIOFE_SPEECH_ENH */
