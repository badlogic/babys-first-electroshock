#include <stdio.h>
#include "mcugdx.h"

#define RED 0b1111100000000000
#define GREEN 0b11111100000
#define BLUE 0b11111
#define BLACK 0x0
#define WHITE 0xffff

// Little blue fucker
#if 0
mcugdx_display_config_t display_config = {
		.driver = MCUGDX_ST7789,
		.native_width = 240,
		.native_height = 320,
		.mosi = 3,
		.sck = 4,
		.dc = 2,
		.cs = 1};
#else
// ILI9341 2,8" 240x320
mcugdx_display_config_t display_config = {
		.driver = MCUGDX_ILI9341,
		.native_width = 240,
		.native_height = 320,
		.mosi = 3,
		.sck = 4,
		.dc = 2,
		.cs = 1,
};
#endif

struct box_t {
	int x, y, vx, vy;
	int width, height;
	uint16_t color;

	void update() {
		if (x + vx > mcugdx_display_width()) {
			vx = -1;
			x = mcugdx_display_width();
		}
		if (x + vx < 0) {
			vx = 1;
			x = 0;
		}
		if (y + vy > mcugdx_display_height()) {
			vy = -1;
			y = mcugdx_display_height();
		}
		if (y + vy < 0) {
			vy = 1;
			y = 0;
		}
		x += vx;
		y += vy;
	}
};

extern "C" void app_main() {
	mcugdx_print_memory();
	mcugdx_init(&display_config);
	mcugdx_print_memory();
	mcugdx_display_set_orientation(MCUGDX_LANDSCAPE);

	box_t boxes[] = {
			{.x = 0, .y = 0, .vx = 1, .vy = 0, .width = 30, .height = 30, .color = RED},
			{.x = mcugdx_display_width() - 30, .y = mcugdx_display_height() - 30, .vx = -1, .vy = 0, .width = 30, .height = 30, .color = BLUE}};
	int frame = 0;
	while (true) {
		double start_time = mcugdx_time();
		mcugdx_display_clear_color(BLACK);

		for (int i = 0; i < 2; i++) {
			box_t *box = &boxes[i];
			box->update();
			mcugdx_display_rect(box->x, box->y, box->width, box->height, box->color);
		}

		double show_time = mcugdx_time();
		mcugdx_display_show();
		frame++;
		if (frame % 30 == 0) {
			double time = mcugdx_time();
			double draw = show_time - start_time;
			double show = time - show_time;
			double total = time - start_time;
			printf("draw: %i ms, show: %i ms, total: %i ms\n", (int) (draw * 1000), (int) (show * 1000), (int) (total * 1000));
		}
	}
}