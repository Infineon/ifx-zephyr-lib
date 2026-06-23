/*
 * Copyright (c) 2026 Infineon Technologies AG
 * SPDX-License-Identifier: Apache-2.0
 *
 * AFE audio process stubs and debug output definitions for basic mode.
 * Provides minimal definitions when speech enhancement and USB debug output are not needed.
 */

#ifndef CY_AFE_AUDIO_PROCESS_STUBS_H__
#define CY_AFE_AUDIO_PROCESS_STUBS_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CONFIG_AUDIOFE_SPEECH_ENH

/* AFE USB Debug Output channel selection constants */
#define AFE_USB_SELECT_AEC_REF     0
#define AFE_USB_SELECT_INPUT_0     1
#define AFE_USB_SELECT_INPUT_1     2
#define AFE_USB_SELECT_OUTPUT      3
#define AFE_USB_SELECT_SIG_A_0     4
#define AFE_USB_SELECT_SIG_A_1     5
#define AFE_USB_SELECT_SIG_B       6
#define AFE_USB_SELECT_SIG_C       7

/* AFE USB Settings structure for debug output */
typedef struct {
    uint32_t channel_0;
    uint32_t channel_1;
    uint32_t channel_2;
} afe_usb_settings_t;

/* Global AFE USB settings variable (stub) */
extern afe_usb_settings_t MY_AFE_USB_SETTINGS;

/* cy_rtos stub functions for queue operations */
static inline cy_rslt_t cy_rtos_queue_count(void *queue, uint32_t *count) {
    if (count) *count = 0;
    return CY_RSLT_SUCCESS;
}

/* cy_rtos stub functions for mutex operations */
static inline cy_rslt_t cy_rtos_mutex_get(void *mutex, uint32_t timeout) {
    (void)mutex;
    (void)timeout;
    return CY_RSLT_SUCCESS;
}

static inline cy_rslt_t cy_rtos_mutex_set(void *mutex) {
    (void)mutex;
    return CY_RSLT_SUCCESS;
}

#endif /* !CONFIG_AUDIOFE_SPEECH_ENH */

#ifdef __cplusplus
}
#endif

#endif /* CY_AFE_AUDIO_PROCESS_STUBS_H__ */
