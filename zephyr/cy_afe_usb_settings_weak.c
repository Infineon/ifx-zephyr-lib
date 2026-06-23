/*
 * Copyright (c) 2026 Infineon Technologies AG
 * SPDX-License-Identifier: Apache-2.0
 *
 * AFE USB Settings Weak Definition
 * Provides a default MY_AFE_USB_SETTINGS for basic mode module builds.
 * The app's configurator_settings.c can override this with strong linkage.
 */

#include <stdint.h>

#ifndef CONFIG_AUDIOFE_SPEECH_ENH

/* Forward declarations matching cy_afe_configurator_settings.h */
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

typedef struct afe_usb_settings {
    afe_usb_select_t channel_0;
    afe_usb_select_t channel_1;
    afe_usb_select_t channel_2;
    afe_usb_select_t channel_3;
} afe_usb_settings_t;

/* Weak definition - will be overridden by app's configurator_settings.c */
__attribute__((weak))
afe_usb_settings_t MY_AFE_USB_SETTINGS = {
    .channel_0 = AFE_USB_SELECT_INPUT_0,
    .channel_1 = AFE_USB_SELECT_INPUT_1,
    .channel_2 = AFE_USB_SELECT_OUTPUT,
    .channel_3 = AFE_USB_SELECT_AEC_REF
};

#endif /* !CONFIG_AUDIOFE_SPEECH_ENH */
