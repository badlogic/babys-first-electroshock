#include <stdio.h>
#include "mcugdx.h"
#include "doomgeneric/doomgeneric.h"
#include "doomgeneric/w_file.h"

#define TAG "DOOM"

// Big & Little blue fucker
#if 1
mcugdx_display_config_t display_config = {
		.driver = MCUGDX_ST7789,
		.native_width = 200,
		.native_height = 320,
		.mosi = 3,
		.sck = 4,
		.dc = 2,
		.cs = 1,
		.reset = 5};
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
		.reset = 11};
#endif

typedef struct {
	wad_file_t wad;
	mcugdx_file_handle_t handle;
} rofs_wad_file_t;

extern wad_file_class_t rofs_wad_file;

static wad_file_t *doom_rofs_open_file(char *path) {
	if (!mcugdx_rofs_exists(path)) return NULL;

	rofs_wad_file_t *result;
	result = mcugdx_mem_alloc(sizeof(rofs_wad_file_t), MCUGDX_MEM_EXTERNAL);
	result->handle = mcugdx_rofs_open(path);
	result->wad.file_class = &rofs_wad_file;
	result->wad.mapped = NULL;
	result->wad.length = mcugdx_rofs_length(result->handle);
	return &result->wad;
}

static void doom_rofs_close_file(wad_file_t *wad) {
	rofs_wad_file_t *rofs_wad;
	rofs_wad = (rofs_wad_file_t *) wad;
	mcugdx_mem_free(rofs_wad);
}

size_t doom_rofs_read(wad_file_t *wad, unsigned int offset,
					  void *buffer, size_t buffer_len) {
	rofs_wad_file_t *rofs_wad = (rofs_wad_file_t *) wad;
	return mcugdx_rofs_read(rofs_wad->handle, offset, buffer, buffer_len);
}

wad_file_class_t rofs_wad_file = {
		doom_rofs_open_file,
		doom_rofs_close_file,
		doom_rofs_read,
};

void DG_Init() {
	DG_ScreenBuffer = (pixel_t *)mcugdx_display_frame_buffer();
}

void DG_SetWindowTitle(const char *title) {
}

int frames = 0;
double last_frame_time = -1;
void DG_DrawFrame(void) {
    mcugdx_display_show();

	frames++;
	if (frames > 60 * 5) {
		frames = 0;
		if (last_frame_time != -1) {
			double frame_time = (mcugdx_time() - last_frame_time) * 1000;
			mcugdx_log(TAG, "Frame time: %f ms, %f fps", frame_time, 1000 / frame_time);
		}
	}
	last_frame_time = mcugdx_time();
}

uint32_t DG_GetTicksMs() {
	uint32_t ticks = (uint32_t)(mcugdx_time() * 1000);
	return ticks;
}

void DG_SleepMs(uint32_t ms) {
	mcugdx_sleep(ms);
}

int DG_GetKey(int *pressed, unsigned char *doomKey) {
	return 0;
}
void app_main() {
	mcugdx_init();
	mcugdx_display_init(&display_config);
	mcugdx_display_set_orientation(MCUGDX_LANDSCAPE);
	mcugdx_rofs_init();

	char *args[] = {"doomgeneric", "-iwad", "Doom1.WAD", "-mmap"};
	doomgeneric_Create(4, args);
	mcugdx_log(TAG, "Game created");
	while (true) {
		doomgeneric_Tick();
	}
}