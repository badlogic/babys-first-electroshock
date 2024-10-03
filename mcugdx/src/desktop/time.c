#include "time.h"
#include <SDL.h>

Uint64 mcugdx_start_time;

double mcugdx_time(void) {
    Uint64 now = SDL_GetPerformanceCounter();
    return (double)(now - mcugdx_start_time) / SDL_GetPerformanceFrequency();
}

void mcugdx_sleep(uint32_t milliseconds) {
    SDL_Delay(milliseconds);
}
