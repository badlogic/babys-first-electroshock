#include "mcugdx.h"
#include <stdio.h>
#include "MiniFB.h"
#include "MiniFB_enums.h"

struct mfb_timer *timer;

void mcugdx_init(mcugdx_display_config_t *display_cfg) {
    timer = mfb_timer_create();
    mcugdx_display_init(display_cfg);
}

double mcugdx_time(void) {
    return mfb_timer_delta(timer);
}

void mcugdx_print_memory(void) {
    printf("mcugdx_print_memory: not implemented on desktop");
}

extern void app_main(void);

int main(void) {
    app_main();
}
