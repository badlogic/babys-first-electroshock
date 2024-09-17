#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "log.h"
#include "mem.h"
#include "mutex.h"
#include "files.h"
#include "image.h"
#include "audio.h"
#include "display.h"
#include "ultrasonic.h"

void mcugdx_init(void);

double mcugdx_time(void);

void mcugdx_sleep(uint32_t millies);

#ifdef __cplusplus
}
#endif
