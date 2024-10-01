#include "display.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>
#include <SDL.h>

#define TAG "mcugdx_display"

#define DEFAULT_FPS 60.0f
static float frame_delay = 1000.0f / DEFAULT_FPS;

static uint32_t *frame_buffer_32;
static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;
mcugdx_display_t display;

extern size_t internal_mem;

extern void mcugdx_desktop_update_button(SDL_KeyboardEvent *event);

mcugdx_result_t mcugdx_display_init(mcugdx_display_config_t *display_cfg) {
	display.native_width = display.width = display_cfg->native_width;
	display.native_height = display.height = display_cfg->native_height;
	display.orientation = MCUGDX_PORTRAIT;
	display.frame_buffer = calloc(display.width * display.height, sizeof(uint16_t));
	frame_buffer_32 = calloc(display.width * display.height, sizeof(uint32_t));

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		mcugdx_loge(TAG, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return MCUGDX_ERROR;
	}

	mcugdx_display_set_orientation(MCUGDX_PORTRAIT);
	return (window && renderer && texture) ? MCUGDX_OK : MCUGDX_ERROR;
}

void mcugdx_display_set_orientation(mcugdx_display_orientation_t orientation) {
	if (orientation == MCUGDX_LANDSCAPE) {
		display.width = display.native_height;
		display.height = display.native_width;
	} else {
		display.width = display.native_width;
		display.height = display.native_height;
	}

	if (window) {
		SDL_DestroyTexture(texture);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
	}

	window = SDL_CreateWindow("mcugdx", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
							  display.width * 2, display.height * 2, SDL_WINDOW_SHOWN);
	if (!window) {
		mcugdx_loge(TAG, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) {
		mcugdx_loge(TAG, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
		return;
	}

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
								display.width, display.height);
	if (!texture) {
		mcugdx_loge(TAG, "Texture could not be created! SDL_Error: %s\n", SDL_GetError());
		return;
	}
}

void mcugdx_display_show(void) {
	Uint32 frame_start, frame_time;
	frame_start = SDL_GetTicks();

	uint32_t *dst = frame_buffer_32;
	uint16_t *src = display.frame_buffer;
	int num_pixels = display.width * display.height;

	for (int i = 0; i < num_pixels; i++) {
		uint16_t pixel = SDL_Swap16(src[i]);

		uint8_t r = (pixel >> 11) & 0x1F;
		uint8_t g = (pixel >> 5) & 0x3F;
		uint8_t b = pixel & 0x1F;

		uint8_t r8 = (r << 3) | (r >> 2);
		uint8_t g8 = (g << 2) | (g >> 4);
		uint8_t b8 = (b << 3) | (b >> 2);

		dst[i] = (0xFF << 24) | (r8 << 16) | (g8 << 8) | b8;
	}

	SDL_UpdateTexture(texture, NULL, frame_buffer_32, display.width * sizeof(uint32_t));
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			exit(0);
		} else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
			mcugdx_desktop_update_button(&event.key);
		}
	}

	frame_time = SDL_GetTicks() - frame_start;
	if (frame_delay > frame_time) {
		// SDL_Delay(frame_delay - frame_time);
	}
}

void mcugdx_display_cleanup(void) {
	free(display.frame_buffer);
	free(frame_buffer_32);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
