#include "audio.h"
#include "log.h"
#include "mutex.h"
#define QOA_IMPLEMENTATION
#define QOA_NO_STDIO
#include "qoa.h"
#include <string.h>
#include <mcugdx.h>
#include <math.h>

#define TAG "mcugdx_audio"

#define MAX_SOUND_INSTANCES 32

static uint32_t next_id = 0;

typedef struct {
	mcugdx_sound_t *sound;
	uint8_t volume;
	uint8_t pan;
	mcugdx_playback_mode mode;
	uint32_t position;
	uint32_t id;
} mcugdx_sound_instance_t;

static mcugdx_sound_instance_t sound_instances[MAX_SOUND_INSTANCES] = {0};
uint8_t master_volume = 128;
extern mcugdx_mutex_t audio_lock;

mcugdx_sound_t *mcugdx_sound_load(const char *path, mcugdx_file_system_t *fs, mcugdx_memory_type_t mem_type) {
	uint32_t size;
	uint8_t *raw = fs->read_fully(path, &size, MCUGDX_MEM_EXTERNAL);
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

mcugdx_sound_id_t mcugdx_sound_play(mcugdx_sound_t *sound, uint8_t volume, uint8_t pan, mcugdx_playback_mode mode) {
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
	instance->pan = pan;
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

bool mcugdx_sound_is_playing(mcugdx_sound_id_t sound_instance) {
	if (sound_instance > MAX_SOUND_INSTANCES) return false;
	mcugdx_mutex_lock(&audio_lock);
	bool is_playing = sound_instances[sound_instance].sound != NULL;
	mcugdx_mutex_unlock(&audio_lock);
	return is_playing;
}

#define FIXED_POINT_BITS 16
#define FIXED_POINT_SCALE (1 << FIXED_POINT_BITS)
#define FIXED_POINT_MASK (FIXED_POINT_SCALE - 1)

int32_t fixed_sqrt(int32_t x) {
    int32_t root = 0;
    int32_t bit = 1 << 30;

    while (bit > x)
        bit >>= 2;

    while (bit != 0) {
        if (x >= root + bit) {
            x -= root + bit;
            root = (root >> 1) + bit;
        } else {
            root >>= 1;
        }
        bit >>= 2;
    }
    return root;
}

void calculate_pan_gains(uint8_t pan, int32_t *left_gain, int32_t *right_gain) {
    *left_gain = (255 - pan) * 256;
    *right_gain = pan * 256;
}

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

    // Adjust volume boost (this gives 1.5x boost)
    int32_t volume_boost = FIXED_POINT_SCALE + (FIXED_POINT_SCALE / 2);

    for (int i = 0; i < MAX_SOUND_INSTANCES; i++) {
        mcugdx_sound_instance_t *instance = &sound_instances[i];

        if (instance->sound == NULL) {
            continue;
        }

        uint32_t frames_to_mix = num_frames;
        uint32_t source_position = instance->position;
        int32_t instance_volume = instance->volume;
        int32_t final_gain = (instance_volume * volume_boost) >> (8 + FIXED_POINT_BITS - 8);
        int32_t pan_left_gain, pan_right_gain;
        calculate_pan_gains(instance->pan, &pan_left_gain, &pan_right_gain);

        while (frames_to_mix > 0) {
            uint32_t frames_left_in_sound = instance->sound->num_frames - source_position;
            uint32_t frames_to_process = (frames_to_mix < frames_left_in_sound) ? frames_to_mix : frames_left_in_sound;

            for (uint32_t frame = 0; frame < frames_to_process; frame++) {
                int32_t left_sample, right_sample;

                if (instance->sound->channels == 1) {
                    int32_t mono_sample = instance->sound->frames[source_position];
                    left_sample = right_sample = mono_sample;
                } else {
                    left_sample = instance->sound->frames[source_position * 2];
                    right_sample = instance->sound->frames[source_position * 2 + 1];
                }

                left_sample = ((left_sample * pan_left_gain) >> 8) * final_gain >> 16;
                right_sample = ((right_sample * pan_right_gain) >> 8) * final_gain >> 16;

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

    int32_t master_volume = mcugdx_audio_get_master_volume();
    int32_t max_amplitude = 0;

    // First pass: find max amplitude
    for (uint32_t i = 0; i < num_frames * channels; i++) {
        int32_t sample = (frames[i] * master_volume) >> 8;
        int32_t abs_sample = sample < 0 ? -sample : sample;
        if (abs_sample > max_amplitude) max_amplitude = abs_sample;
    }

    // Calculate scaling factor if necessary
    float scale = 1.0f;
    if (max_amplitude > INT16_MAX) {
        scale = (float)INT16_MAX / max_amplitude;
    }

    // Second pass: apply scaling and convert to int16_t
    int16_t *output = (int16_t *)frames;
    for (uint32_t i = 0; i < num_frames * channels; i++) {
        int32_t sample = (frames[i] * master_volume) >> 8;
        sample = (int32_t)(sample * scale);

        if (sample > INT16_MAX) sample = INT16_MAX;
        else if (sample < INT16_MIN) sample = INT16_MIN;

        output[i] = (int16_t)sample;
    }
}

void mcugdx_audio_set_master_volume(uint8_t volume) {
	master_volume = volume;
}

uint8_t mcugdx_audio_get_master_volume(void) {
	return master_volume;
}
