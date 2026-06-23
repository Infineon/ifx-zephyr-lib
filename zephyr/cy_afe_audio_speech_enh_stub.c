/*
 * Copyright (c) 2026 Infineon Technologies AG
 * SPDX-License-Identifier: Apache-2.0
 *
 * AFE Speech Enhancement Stub Implementation
 * Provides empty implementations when CONFIG_AUDIOFE_SPEECH_ENH is not enabled.
 */

#include "cy_result.h"
#include <stdint.h>
#include <stdbool.h>

#ifndef CONFIG_AUDIOFE_SPEECH_ENH

/* Stub implementations of speech enhancement functions
 * These are called by the AFE core but do nothing when speech enhancement is disabled.
 */

cy_rslt_t afe_speech_enhancement_init(
    int32_t *filter_settings,
    uint8_t *mw_settings,
    uint16_t mw_settings_length,
    void *context)
{
    /* Stub: do nothing for basic AFE mode */
    (void)filter_settings;
    (void)mw_settings;
    (void)mw_settings_length;
    (void)context;
    return CY_RSLT_SUCCESS;
}

cy_rslt_t afe_speech_enhancement_process(
    void *context,
    void *sp_enh_input_output)
{
    /* Stub: do nothing for basic AFE mode */
    (void)context;
    (void)sp_enh_input_output;
    return CY_RSLT_SUCCESS;
}

cy_rslt_t afe_speech_enhancement_deinit(void *context)
{
    /* Stub: do nothing for basic AFE mode */
    (void)context;
    return CY_RSLT_SUCCESS;
}

cy_rslt_t afe_speech_enhancement_enable_disable_component(
    void *context,
    int component_name,
    bool enable)
{
    /* Stub: do nothing for basic AFE mode */
    (void)context;
    (void)component_name;
    (void)enable;
    return CY_RSLT_SUCCESS;
}

cy_rslt_t afe_speech_enhancement_update_config_value(
    void *context,
    int config_name,
    int32_t value)
{
    /* Stub: do nothing for basic AFE mode */
    (void)context;
    (void)config_name;
    (void)value;
    return CY_RSLT_SUCCESS;
}

cy_rslt_t afe_speech_enhancement_get_config_value(
    void *context,
    int component_name,
    int32_t *value)
{
    /* Stub: return default value */
    (void)context;
    (void)component_name;
    if (value) *value = 0;
    return CY_RSLT_SUCCESS;
}

cy_rslt_t afe_speech_enhancement_get_component_status(
    void *context,
    int component_name,
    bool *enable)
{
    /* Stub: report disabled */
    (void)context;
    (void)component_name;
    if (enable) *enable = false;
    return CY_RSLT_SUCCESS;
}

cy_rslt_t afe_speech_enhancement_update_dbg_out_config(
    void *sp_enh_context,
    void *pMY_AFE_USB_SETTINGS)
{
    /* Stub: do nothing for basic AFE mode */
    (void)sp_enh_context;
    (void)pMY_AFE_USB_SETTINGS;
    return CY_RSLT_SUCCESS;
}

cy_rslt_t afe_speech_enhancement_get_sound_meter(
    void *context,
    int16_t *audio_meter)
{
    /* Stub: return default meter value */
    (void)context;
    if (audio_meter) *audio_meter = 0;
    return CY_RSLT_SUCCESS;
}

cy_rslt_t afe_speech_enhancement_get_component_params(
    void *context,
    int component_name,
    void *params)
{
    /* Stub: do nothing for basic AFE mode */
    (void)context;
    (void)component_name;
    (void)params;
    return CY_RSLT_SUCCESS;
}

#endif /* !CONFIG_AUDIOFE_SPEECH_ENH */
