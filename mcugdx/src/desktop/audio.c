#include "audio.h"
#include "log.h"
#include "mutex.h"
#define SOKOL_AUDIO_IMPL
#include "sokol_audio.h"

#define TAG "mcugdx_audio"

static uint32_t sample_rate = 0;
static uint32_t channels = 0;
static int32_t *frames;
mcugdx_mutex_t audio_lock;

static void log(
		const char *tag,
		uint32_t log_level,
		uint32_t log_item_id,
		const char *message_or_null,
		uint32_t line_nr,
		const char *filename_or_null,
		void *user_data) {
	(void) log_item_id;
	(void) line_nr;
	(void) filename_or_null;
	(void) user_data;
	if (!message_or_null) return;

	if (log_level == 3) {
		mcugdx_log(tag, message_or_null);
	} else {
		mcugdx_loge(tag, message_or_null);
	}
}

static void mix_and_stream(float *buffer, int num_frames, int num_channels) {
	if(frames == NULL) {
		int buffer_frames = saudio_buffer_frames();
		frames = (int32_t *) mcugdx_mem_alloc(sizeof(int32_t) * buffer_frames * channels, MCUGDX_MEM_EXTERNAL);
	}

	mcugdx_audio_mix(frames, num_frames, num_channels);
	int16_t *frames_i16 = (int16_t *) frames;
	for (int i = 0; i < num_frames * num_channels; i++) {
		buffer[i] = frames_i16[i] / 32768.0f;
	}
}

mcugdx_result_t mcugdx_audio_init(mcugdx_audio_config_t *config) {
	sample_rate = config->sample_rate;
	channels = config->channels;

	if (!mcugdx_mutex_init(&audio_lock)) {
		mcugdx_loge(TAG, "Could not create audio lock");
		return MCUGDX_ERROR;
	}

	saudio_setup(&(saudio_desc){
			.num_channels = config->channels,
			.sample_rate = config->sample_rate,
			.logger = {.func = log},
			.stream_cb = mix_and_stream});

	if (saudio_sample_rate() != config->sample_rate) {
		mcugdx_mutex_destroy(&audio_lock);
		mcugdx_loge(TAG, "Could not initiated audio device with sample rate %i and %i channels. Got %i sample rate and %i channels", config->sample_rate, config->channels, saudio_sample_rate(), saudio_channels());
		return MCUGDX_ERROR;
	}

	mcugdx_log(TAG, "Initialized audio device, sample rate: %i, channels: %i, buffer size: %i frames", sample_rate, channels, saudio_buffer_frames());

	return MCUGDX_OK;
}

uint32_t mcugdx_audio_get_sample_rate(void) {
	return sample_rate;
}
