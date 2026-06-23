#ifndef CY_AUDIO_LICENSE_H_
#define CY_AUDIO_LICENSE_H_

#include <stdbool.h>

#define CY_AUDIO_LICENSE_VALID 1

static inline int cy_audio_license_check(void)
{
    return CY_AUDIO_LICENSE_VALID;
}

static inline bool cy_afe_lib_is_license_expired(void)
{
    return false;
}

#endif
