/*
 * Copyright (c) 2026 Infineon Technologies AG
 * SPDX-License-Identifier: Apache-2.0
 *
 * Shadow ethosu_driver.h — API compatibility shim for COMPONENT_U55 in the
 * Infineon ML Middleware.
 *
 * Problem:
 *   The ethos-u-core-driver in this workspace uses a newer API for its
 *   platform cache callbacks (3 parameters, const uint64_t *):
 *     void ethosu_flush_dcache(const uint64_t *base_addr,
 *                              const size_t   *base_addr_size,
 *                              int             num_base_addr);
 *
 *   The ml-middleware's mtb_ml_ethosu.c implements the older 2-parameter API:
 *     void ethosu_flush_dcache(uint32_t *p, size_t bytes);
 *
 *   This version mismatch causes a "conflicting types" compile error.
 *
 * Solution (Option A — shadow header):
 *   This file is placed in a directory that is added to the include search
 *   path BEFORE the real ethos_u HAL include, but ONLY for the
 *   mtb_ml_ethosu.c compilation unit (via set_source_files_properties
 *   COMPILE_OPTIONS "-I<compat_dir>").
 *
 *   Before pulling in the real ethosu_driver.h (via #include_next), the two
 *   conflicting function names are temporarily macro-renamed to harmless
 *   names. The real header's declarations then use those renamed symbols.
 *   After the real header is processed, the macros are un-defined, so the
 *   definitions in mtb_ml_ethosu.c use the original (unmangled) names and
 *   compile without conflict.
 *
 * Scope:
 *   Applied only to mtb_ml_ethosu.c. No other TU sees this shadow.
 */

/* Rename the new 3-param cache callback declarations so they do not
 * conflict with the old 2-param definitions in mtb_ml_ethosu.c. */
#define ethosu_flush_dcache      _unused_ethosu_flush_dcache_v2
#define ethosu_invalidate_dcache _unused_ethosu_invalidate_dcache_v2

/* Pull in the real ethosu_driver.h from the next directory in the include
 * search path (skips this shadow directory). The cache declarations above
 * are preprocessed into their renamed forms and cause no conflict. */
#include_next <ethosu_driver.h>

/* Restore the original names. The definitions in mtb_ml_ethosu.c will
 * use these unmangled names — correct behaviour. */
#undef ethosu_flush_dcache
#undef ethosu_invalidate_dcache
