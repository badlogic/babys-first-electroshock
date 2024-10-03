#include "mcugdx.h"

#define TAG "Audio example"

extern "C" void app_main() {
	mcugdx_init();
	mcugdx_rofs_init();

	mcugdx_audio_config_t audio_config = {
			.sample_rate = 44100,
			.channels = MCUGDX_STEREO,
			.bclk = 47,
			.ws = 21,
			.dout = 38};
	mcugdx_audio_init(&audio_config);
	mcugdx_audio_set_master_volume(128);

	mcugdx_sound_t *sound = mcugdx_sound_load("synth.qoa", &mcugdx_rofs, MCUGDX_MEM_EXTERNAL);
	if (sound == NULL) {
		mcugdx_log(TAG, "Failed to load sound");
		return;
	}

	for (int i = 0; i < 1; i++) {
		mcugdx_sound_play(sound, 255, 127, MCUGDX_LOOP);
	}

	mcugdx_mem_print();

    mcugdx_display_config_t display_config = {
		.driver = MCUGDX_ST7789,
		.native_width = 240,
		.native_height = 320,
		.mosi = 3,
		.sck = 4,
		.dc = 2,
		.cs = 1,
		.reset = -1};
    mcugdx_display_init(&display_config);

    int frame = 0;
	while (true) {
        double start = mcugdx_time();
        mcugdx_display_clear_color(MCUGDX_PINK);
		mcugdx_display_show();

        frame++;
		if (frame % 30 == 0) {
			double time = mcugdx_time();
			double total = time - start;
			mcugdx_log(TAG, "total: %.3f ms", total * 1000);
		}
	}
}
