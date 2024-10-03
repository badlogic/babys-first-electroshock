#include <stdio.h>
#include "mcugdx.h"

#define TAG "display test"

#define RED 0b1111100000000000
#define GREEN 0b11111100000
#define BLUE 0b11111
#define BLACK 0x0
#define WHITE 0xffff
#define PINK 0b1111100000011111

// Big & Little blue fucker
#if 1
mcugdx_display_config_t display_config = {
		.driver = MCUGDX_ST7789,
		.native_width = 240,
		.native_height = 320,
		.mosi = 3,
		.sck = 4,
		.dc = 2,
		.cs = 1,
		.reset = 17};
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
		.reset = 17
};
#endif

struct box_t {
	int x, y, vx, vy;
	int width, height;
	uint16_t color;
	mcugdx_image_t *image;

	void update() {
		if (x + vx > mcugdx_display_width() - width) {
			vx = -1;
			x = mcugdx_display_width() - width;
		}
		if (x + vx < 0) {
			vx = 1;
			x = 0;
		}
		if (y + vy > mcugdx_display_height() - height) {
			vy = -1;
			y = mcugdx_display_height() - height;
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
	mcugdx_mem_print();
	mcugdx_init();
	mcugdx_display_init(&display_config);
	mcugdx_mem_print();
	mcugdx_display_set_orientation(MCUGDX_LANDSCAPE);
	mcugdx_rofs_init();

	mcugdx_mem_print();

	double load_start = mcugdx_time();
	mcugdx_image_t *bear = mcugdx_image_load("bear.qoi", &mcugdx_rofs, MCUGDX_MEM_INTERNAL);
	mcugdx_log(TAG, "Load took: %f", (mcugdx_time() - load_start));
	mcugdx_mem_print();

	box_t boxes[] = {
			{.x = 0, .y = 0, .vx = 1, .vy = 0, .width = 30, .height = 30, .color = RED, .image = NULL},
			{.x = mcugdx_display_width() / 2 - 15, .y = mcugdx_display_height() / 2 - 60, .vx = -1, .vy = 0, .width = 30, .height = 30, .color = GREEN, .image = NULL},
			{.x = 0, .y = 0, .vx = 1, .vy = 1, .width = (int)bear->width, .height = (int)bear->height, .color = GREEN, .image = bear},
			{.x = mcugdx_display_width() - 30, .y = mcugdx_display_height() - 30, .vx = -1, .vy = 0, .width = 30, .height = 30, .color = BLUE, .image = NULL}};
	int frame = 0;
	while (true) {
		double start_time = mcugdx_time();
		mcugdx_display_clear_color(PINK);
		double end_clear = mcugdx_time();

		for (int i = 0; i < 4; i++) {
			box_t *box = &boxes[i];
			box->update();
			if (!box->image) {
				mcugdx_display_rect(box->x, box->y, box->width, box->height, box->color);
			} else {
				for (int j = 0; j < 1; j++)
				// mcugdx_display_blit(box->image, 0, 0);
					mcugdx_display_blit_keyed(box->image, box->x, box->y, 0x0);
			}
		}

		double show_time = mcugdx_time();
		mcugdx_display_show();
		frame++;
		if (frame % 30 == 0) {
			double time = mcugdx_time();
			double clear = end_clear - start_time;
			double draw = show_time - start_time;
			double show = time - show_time;
			double total = time - start_time;
			mcugdx_log(TAG, "clear: %.3f, draw: %.3f ms, show: %.3f ms, total: %.3f ms", (clear * 1000), (draw * 1000), (show * 1000), (total * 1000));
		}
	}
}