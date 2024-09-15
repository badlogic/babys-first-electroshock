#include "audio.h"
#include "log.h"
#include "mutex.h"
#define QOA_IMPLEMENTATION
#define QOA_NO_STDIO
#include "qoa.h"
#include <string.h>
#include <mcugdx.h>

#define TAG "mcugdx_audio"

#define MAX_SOUND_INSTANCES 32

static uint32_t next_id = 0;

typedef struct {
	mcugdx_sound_t *sound;
	uint8_t volume;
	mcugdx_playback_mode mode;
	uint32_t position;
	uint32_t id;
} mcugdx_sound_instance_t;

static mcugdx_sound_instance_t sound_instances[MAX_SOUND_INSTANCES] = {0};
uint8_t master_volume = 128;

extern mcugdx_mutex_t audio_lock;

static inline uint32_t isqrt(uint32_t x) {
	uint32_t res = 0;
	uint32_t bit = 1 << 30;

	while (bit > x)
		bit >>= 2;

	while (bit != 0) {
		if (x >= res + bit) {
			x -= res + bit;
			res = (res >> 1) + bit;
		} else
			res >>= 1;
		bit >>= 2;
	}
	return res;
}

static inline uint8_t scale_volume(uint8_t volume) {
	uint32_t scaled = isqrt((uint32_t) volume * volume);
	return (uint8_t) (scaled > 255 ? 255 : scaled);
}

mcugdx_sound_t *mcugdx_sound_load(const char *path, mcugdx_read_file_func_t read_file, mcugdx_memory_type_t mem_type) {
	uint32_t size;
	uint8_t *raw = read_file(path, &size, MCUGDX_MEM_EXTERNAL);
	qoa_desc qoa;

	if (!qoa_decode_header(raw, size, &qoa)) {
		mcugdx_loge(TAG, "Could not load sound %s, invalid header\n", path);
		mcugdx_mem_free(raw);
		return NULL;
	}

	if (qoa.samplerate != mcugdx_audio_get_sample_rate()) {
		mcugdx_loge(TAG, "Sampling rate of sound file %li != audio system sampling rate %li\n", qoa.samplerate, mcugdx_audio_get_sample_rate());
		mcugdx_mem_free(raw);
		return NULL;
	}

	int16_t *frames = qoa_decode(raw, size, &qoa, mem_type);
	if (!frames) {
		mcugdx_loge(TAG, "Could not load sound %s, invalid data\n", path);
		mcugdx_mem_free(raw);
		return NULL;
	}

	mcugdx_sound_t *sound = (mcugdx_sound_t *) mcugdx_mem_alloc(sizeof(mcugdx_sound_t), mem_type);
	sound->channels = qoa.channels;
	sound->frames = frames;
	sound->sample_rate = qoa.samplerate;
	sound->num_frames = qoa.samples;

	mcugdx_mem_free(raw);

	return sound;
}

void mcugdx_sound_unload(mcugdx_sound_t *sound) {
	(void) sound;
}

mcugdx_sound_id_t mcugdx_sound_play(mcugdx_sound_t *sound, uint8_t volume, mcugdx_playback_mode mode) {
	mcugdx_mutex_lock(&audio_lock);
	mcugdx_sound_instance_t *free_slot = NULL;
	mcugdx_sound_id_t free_slot_idx = 0;
	mcugdx_sound_instance_t *lowest_id_slot = &sound_instances[0];
	mcugdx_sound_id_t lowest_id_slot_idx = 0;

	for (int i = 0; i < MAX_SOUND_INSTANCES; i++) {
		if (sound_instances[i].sound == NULL) {
			free_slot = &sound_instances[i];
			free_slot_idx = i;
			break;
		}
		if (sound_instances[i].id < lowest_id_slot->id) {
			lowest_id_slot = &sound_instances[i];
			lowest_id_slot_idx = i;
		}
	}

	mcugdx_sound_instance_t *instance = free_slot ? free_slot : lowest_id_slot;
	mcugdx_sound_id_t id = free_slot ? free_slot_idx : lowest_id_slot_idx;

	instance->sound = sound;
	instance->volume = volume;
	instance->mode = mode;
	instance->position = 0;
	instance->id = ++next_id;
	mcugdx_mutex_unlock(&audio_lock);
	return id;
}

void mcugdx_sound_set_volume(mcugdx_sound_id_t sound_instance, uint8_t volume) {
	if (sound_instance > MAX_SOUND_INSTANCES) return;
	mcugdx_mutex_lock(&audio_lock);
	sound_instances[sound_instance].volume = volume;
	mcugdx_mutex_unlock(&audio_lock);
}

void mcugdx_sound_stop(mcugdx_sound_id_t sound_instance) {
	if (sound_instance > MAX_SOUND_INSTANCES) return;
	mcugdx_mutex_lock(&audio_lock);
	sound_instances[sound_instance].sound = NULL;
	mcugdx_mutex_unlock(&audio_lock);
}

#define FIXED_POINT_BITS 8
#define FIXED_POINT_SCALE (1 << FIXED_POINT_BITS)

void mcugdx_audio_mix(int32_t *frames, uint32_t num_frames, mcugdx_audio_channels_t channels) {
	memset(frames, 0, num_frames * channels * sizeof(int32_t));

	mcugdx_mutex_lock(&audio_lock);

	uint32_t active_sources = 0;
	for (int i = 0; i < MAX_SOUND_INSTANCES; i++) {
		if (sound_instances[i].sound != NULL) {
			active_sources++;
		}
	}

	if (active_sources == 0) {
		mcugdx_mutex_unlock(&audio_lock);
		return;
	}

	int32_t source_scale = (FIXED_POINT_SCALE + active_sources - 1) / active_sources;

	for (int i = 0; i < MAX_SOUND_INSTANCES; i++) {
		mcugdx_sound_instance_t *instance = &sound_instances[i];

		if (instance->sound == NULL) {
			continue;
		}

		uint32_t frames_to_mix = num_frames;
		uint32_t source_position = instance->position;
		int32_t instance_volume = (scale_volume(instance->volume) * source_scale + 127) / 255;

		while (frames_to_mix > 0) {
			uint32_t frames_left_in_sound = instance->sound->num_frames - source_position;
			uint32_t frames_to_process = (frames_to_mix < frames_left_in_sound) ? frames_to_mix : frames_left_in_sound;

			for (uint32_t frame = 0; frame < frames_to_process; frame++) {
				int32_t left_sample, right_sample;

				if (instance->sound->channels == 1) {
					left_sample = right_sample = instance->sound->frames[source_position];
				} else {
					left_sample = instance->sound->frames[source_position * 2];
					right_sample = instance->sound->frames[source_position * 2 + 1];
				}

				left_sample = (left_sample * instance_volume) >> FIXED_POINT_BITS;
				right_sample = (right_sample * instance_volume) >> FIXED_POINT_BITS;

				if (channels == MCUGDX_MONO) {
					frames[frame] += (left_sample + right_sample) >> 1;
				} else {
					frames[frame * 2] += left_sample;
					frames[frame * 2 + 1] += right_sample;
				}

				source_position++;
			}

			frames_to_mix -= frames_to_process;

			if (source_position >= instance->sound->num_frames) {
				if (instance->mode == MCUGDX_LOOP) {
					source_position = 0;
				} else {
					instance->sound = NULL;
					break;
				}
			}
		}

		instance->position = source_position;
	}

	mcugdx_mutex_unlock(&audio_lock);

	uint8_t master_volume = scale_volume(mcugdx_audio_get_master_volume());

	int16_t *output = (int16_t *) frames;
	for (uint32_t i = 0; i < num_frames * channels; i++) {
		int32_t sample = frames[i];

		sample = (sample * master_volume) / 255;

		if (sample > INT16_MAX) {
			sample = INT16_MAX;
		} else if (sample < INT16_MIN) {
			sample = INT16_MIN;
		}

		output[i] = (int16_t) sample;
	}
}

void mcugdx_audio_set_master_volume(uint8_t volume) {
	master_volume = volume;
}

uint8_t mcugdx_audio_get_master_volume(void) {
	return master_volume;
}
