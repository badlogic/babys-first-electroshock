#include "mcugdx.h"
#include <stdio.h>
#include "MiniFB.h"
#include "MiniFB_enums.h"

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__) || defined(__linux__)
#include <unistd.h>
#else
#error "Unsupported platform"
#endif

struct mfb_timer *timer;

void mcugdx_init(void) {
	timer = mfb_timer_create();
}

double mcugdx_time(void) {
	return mfb_timer_now(timer);
}

void mcugdx_sleep(uint32_t milliseconds) {
#ifdef _WIN32
	Sleep(milliseconds);
#elif defined(__APPLE__) || defined(__linux__)
	usleep(milliseconds * 1000);
#endif
}

extern void app_main(void);

int main(void) {
	app_main();
}
