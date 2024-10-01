#include "mcugdx.h"
#include <SDL.h>
#include <stdio.h>

static Uint64 start_time;

void mcugdx_init(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }
    start_time = SDL_GetPerformanceCounter();
}

double mcugdx_time(void) {
    Uint64 now = SDL_GetPerformanceCounter();
    return (double)(now - start_time) / SDL_GetPerformanceFrequency();
}

void mcugdx_sleep(uint32_t milliseconds) {
    SDL_Delay(milliseconds);
}

extern void app_main(void);

int main(void) {
	app_main();
}
