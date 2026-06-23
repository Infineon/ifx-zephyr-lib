/*
 * Copyright (c) 2026 Infineon Technologies AG
 * SPDX-License-Identifier: Apache-2.0
 *
 * Speech enhancement stub header for basic AFE-only builds.
 * Provides minimal type definitions when CONFIG_AUDIOFE_SPEECH_ENH is not enabled.
 */

#ifndef IFX_SP_ENH_STUB_H__
#define IFX_SP_ENH_STUB_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Minimal stubs for speech enhancement types used by AFE */
typedef struct {
    uint32_t dummy;
} ifx_stc_sp_enh_info_t;

typedef void* ifx_sp_enh_context_t;

/* Empty stub functions that may be referenced during build */
static inline void ifx_sp_enh_init(void) {}
static inline void ifx_sp_enh_process(void) {}
static inline void ifx_sp_enh_deinit(void) {}

#ifdef __cplusplus
}
#endif

#endif /* IFX_SP_ENH_STUB_H__ */
