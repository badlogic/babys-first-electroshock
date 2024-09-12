#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "display.h"

void mcugdx_init(mcugdx_display_config_t *display_config);

double mcugdx_time(void);

void mcugdx_print_memory(void);

#ifdef __cplusplus
}
#endif
