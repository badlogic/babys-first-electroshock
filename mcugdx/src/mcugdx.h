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
#include "neopixels.h"
#include "buttons.h"
#include "time.h"
#include "prefs.h"

void mcugdx_init(void);

#ifdef __cplusplus
}
#endif
