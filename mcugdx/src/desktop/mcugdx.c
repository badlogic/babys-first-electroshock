#include "mcugdx.h"
#include <stdio.h>
#include "MiniFB.h"
#include "MiniFB_enums.h"

struct mfb_timer *timer;

void mcugdx_init(void) {
	timer = mfb_timer_create();
}

double mcugdx_time(void) {
	return mfb_timer_now(timer);
}

extern void app_main(void);

int main(void) {
	app_main();
}
