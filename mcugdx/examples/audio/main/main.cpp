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
	mcugdx_audio_set_master_volume(255);

	mcugdx_log(TAG, "Before load");
	mcugdx_mem_print();

	mcugdx_sound_t *sound = mcugdx_sound_load("synth.qoa", &mcugdx_rofs, MCUGDX_STREAMED, MCUGDX_MEM_EXTERNAL);
	if (sound == NULL) {
		mcugdx_log(TAG, "Failed to load sound");
		return;
	}

	mcugdx_log(TAG, "After load");
	mcugdx_mem_print();

	mcugdx_sound_id_t synth = mcugdx_sound_play(sound, 255, 127, MCUGDX_SINGLE_SHOT);

	mcugdx_log(TAG, "After play");
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
	while (mcugdx_sound_is_playing(synth)) {
        double start = mcugdx_time();
        mcugdx_display_clear_color(MCUGDX_PINK);
		mcugdx_display_show();
	}

	mcugdx_log(TAG, "Before unload");
	mcugdx_mem_print();

	mcugdx_sound_unload(sound);
	mcugdx_log(TAG, "After unload");
	mcugdx_mem_print();
}
