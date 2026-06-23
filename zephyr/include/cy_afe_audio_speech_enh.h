/*
 * Copyright (c) 2026 Infineon Technologies AG
 * SPDX-License-Identifier: Apache-2.0
 *
 * Speech enhancement stub for AFE basic mode (no voice core dependency).
 * This header shadows the real cy_afe_audio_speech_enh.h when CONFIG_AUDIOFE_SPEECH_ENH is not set.
 */

#ifndef AUDIO_FRONT_END_SPEECH_ENHANCEMENT_H__
#define AUDIO_FRONT_END_SPEECH_ENHANCEMENT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cy_result.h"
#include <stdint.h>
#include <stdbool.h>

#ifndef CONFIG_AUDIOFE_SPEECH_ENH

/* ===== Basic AFE Mode: Stub implementations ===== */

/* Minimal type stubs for speech enhancement integration */
typedef struct {
    uint32_t dummy;
} ifx_stc_sp_enh_info_t;

typedef int ifx_sp_enh_ip_component_config_t;

/* Structure used by AFE audio processing even in basic mode */
typedef struct {
    int16_t *input1;
    int16_t *input2;
    int16_t *aec_reference_input;
    int16_t *output;
    int16_t *ifx_internal_output;
} afe_sp_enh_input_output_t;

/* Speech enhancement function declarations (implemented in stub .c file) */
cy_rslt_t afe_speech_enhancement_init(int32_t *filter_settings, uint8_t *mw_settings, uint16_t mw_settings_length, void *context);
cy_rslt_t afe_speech_enhancement_process(void *context, void *sp_enh_input_output);
cy_rslt_t afe_speech_enhancement_deinit(void *context);
cy_rslt_t afe_speech_enhancement_enable_disable_component(void *context, int component_name, bool enable);
cy_rslt_t afe_speech_enhancement_update_config_value(void *context, int config_name, int32_t value);
cy_rslt_t afe_speech_enhancement_get_config_value(void *context, int component_name, int32_t *value);
cy_rslt_t afe_speech_enhancement_get_component_status(void *context, int component_name, bool *enable);
cy_rslt_t afe_speech_enhancement_update_dbg_out_config(void *sp_enh_context, void *pMY_AFE_USB_SETTINGS);
cy_rslt_t afe_speech_enhancement_get_sound_meter(void *context, int16_t *audio_meter);
cy_rslt_t afe_speech_enhancement_get_component_params(void *context, int component_name, void *params);

#else

/* Speech enhancement enabled - include the real implementation
 * This will be resolved from the audio-front-end asset directory
 * when CONFIG_AUDIOFE_SPEECH_ENH is actually defined
 */
#error "Speech enhancement headers not yet supported - set CONFIG_AUDIOFE_SPEECH_ENH=n"

#endif

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_FRONT_END_SPEECH_ENHANCEMENT_H__ */
