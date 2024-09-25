#pragma once

#include <stdint.h>
#include "files.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	MCUGDX_MONO = 1,
	MCUGDX_STEREO = 2
} mcugdx_audio_channels_t;

typedef struct {
	int sample_rate;
	mcugdx_audio_channels_t channels;
	int bclk;
	int ws;
	int dout;
} mcugdx_audio_config_t;

typedef struct {
	uint32_t sample_rate;
	uint32_t channels;
	int16_t *frames;
	uint32_t num_frames;
} mcugdx_sound_t;

typedef enum {
	MCUGDX_SINGLE_SHOT,
	MCUGDX_LOOP
} mcugdx_playback_mode;

typedef uint32_t mcugdx_sound_id_t;

mcugdx_result_t mcugdx_audio_init(mcugdx_audio_config_t *config);

void mcugdx_audio_mix(int32_t *frames, uint32_t num_frames, mcugdx_audio_channels_t channels);

void mcugdx_audio_set_master_volume(uint8_t volume);

uint8_t mcugdx_audio_get_master_volume(void);

uint32_t mcugdx_audio_get_sample_rate(void);

mcugdx_sound_t *mcugdx_sound_load(const char *path, mcugdx_read_file_func_t read_file, mcugdx_memory_type_t mem_type);

void mcugdx_sound_unload(mcugdx_sound_t *sound);

mcugdx_sound_id_t mcugdx_sound_play(mcugdx_sound_t *sound, uint8_t volume, uint8_t pan, mcugdx_playback_mode mode);

void mcugdx_sound_set_volume(mcugdx_sound_id_t sound_instance, uint8_t volume);

void mcugdx_sound_set_volume(mcugdx_sound_id_t sound_instance, uint8_t pan);

void mcugdx_sound_stop(mcugdx_sound_id_t sound_instance);

mcugdx_result_t mcugdx_sound_is_playing(mcugdx_sound_id_t sound_instance);

#ifdef __cplusplus
}
#endif
