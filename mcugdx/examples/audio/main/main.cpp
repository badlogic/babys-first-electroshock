#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
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
	mcugdx_audio_set_master_volume(32);

	mcugdx_sound_t *sound = mcugdx_sound_load("music-stereo.qoa", mcugdx_rofs_read_file, MCUGDX_MEM_EXTERNAL);
	if (sound == NULL) {
		mcugdx_log(TAG, "Failed to load sound");
		return;
	}

	for (int i = 0; i < 1; i++) {
		mcugdx_sound_play(sound, 255, MCUGDX_LOOP);
	}

	mcugdx_mem_print();

	while (true) {
		mcugdx_log(TAG, "main loop");
		mcugdx_sleep(1000);
	}
}
