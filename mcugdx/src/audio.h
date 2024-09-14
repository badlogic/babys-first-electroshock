#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	MONO,
	STEREO
} mcugdx_audio_channels_t;

typedef struct {
	int sampling_frequency;
	mcugdx_audio_channels_t channels;
} mcugdx_audio_config_t;

void mcugdx_audio_init(mcugdx_audio_config_t config);

#ifdef __cplusplus
}
#endif
