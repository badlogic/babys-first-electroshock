#include "mcugdx.h"
#include "stdio.h"
#include "esp_timer.h"

#define TAG "MCUGDX"

void mcugdx_init(mcugdx_display_config_t *display_cfg) {
    mcugdx_display_init(display_cfg);
}

double mcugdx_time() {
    return (double) esp_timer_get_time() / 1000000;
}
