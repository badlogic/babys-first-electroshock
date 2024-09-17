#pragma once

#include "result.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int trigger;
    int echo;
} mcugdx_ultrasonic_config_t;

mcugdx_result_t mcugdx_ultrasonic_init(mcugdx_ultrasonic_config_t *config);

mcugdx_result_t mcugdx_ultrasonic_measure(uint32_t max_distance, uint32_t *distance_cm);

#ifdef __cplusplus
}
#endif
