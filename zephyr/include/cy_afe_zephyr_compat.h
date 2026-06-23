/*
 * Copyright (c) 2026 Infineon Technologies AG
 * SPDX-License-Identifier: Apache-2.0
 *
 * AFE Basic Mode Compatibility Layer
 * Provides cy_rtos stubs for building AFE in basic mode without voice core.
 * This should be included early in the compilation to provide missing RTOS functions.
 */

#ifndef CY_AFE_ZEPHYR_COMPAT_H__
#define CY_AFE_ZEPHYR_COMPAT_H__

/* Guard against inclusion in assembly files */
#if !defined(__ASSEMBLY__)

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CONFIG_AUDIOFE_SPEECH_ENH

#include <stdint.h>
#include "cy_result.h"

/* ===== AFE USB Debug Output Stubs ===== */
/* These enums and globals are referenced by AFE audio processing code
 * for debug output features. Provide stubs here; can be overridden by app.
 */
#ifndef AFE_USB_SELECT_INPUT_0
typedef enum {
    AFE_USB_SELECT_INPUT_0  = 0,
    AFE_USB_SELECT_INPUT_1  = 1,
    AFE_USB_SELECT_OUTPUT   = 2,
    AFE_USB_SELECT_AEC_REF  = 3,
    AFE_USB_SELECT_SIG_A_0  = 4,
    AFE_USB_SELECT_SIG_A_1  = 5,
    AFE_USB_SELECT_SIG_B    = 6,
    AFE_USB_SELECT_SIG_C    = 7
} afe_usb_select_t;

typedef struct {
    uint32_t channel_0;
    uint32_t channel_1;
    uint32_t channel_2;
    uint32_t channel_3;
} afe_usb_settings_t;

extern afe_usb_settings_t MY_AFE_USB_SETTINGS;
#endif

/* ===== cy_rtos Stub Functions (used by AFE audio processing) ===== */
/* These are called by cy_afe_audio_process.c for queue/mutex management
 * when speech enhancement mode is disabled (basic AFE only)
 */

static inline cy_rslt_t cy_rtos_queue_count(void *queue, uint32_t *count) {
    if (count) *count = 0;
    return CY_RSLT_SUCCESS;
}

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

#endif /* !__ASSEMBLY__ */

#endif /* CY_AFE_ZEPHYR_COMPAT_H__ */
