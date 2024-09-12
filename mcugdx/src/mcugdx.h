#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "audio.h"
#include "display.h"
#include "log.h"
#include "mem.h"

void mcugdx_init(mcugdx_display_config_t *display_config);

double mcugdx_time(void);

#ifdef __cplusplus
}
#endif
