#include "mcugdx.h"
#include <SDL.h>
#include <stdio.h>

extern Uint64 mcugdx_start_time;

void mcugdx_init(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }
    mcugdx_start_time = SDL_GetPerformanceCounter();
}

void mcugdx_quit(void) {
    exit(0);
}

extern void app_main(void);

int main(void) {
	app_main();
}
