#ifndef AUDIO_H
#define AUDIO_H

#include <complex.h>
#include <pico/types.h>

#define SAMPLE_COUNT_512 512U
#define SAMPLE_COUNT_1024 1024U
#define SAMPLE_COUNT_2048 2048U
#define SAMPLE_COUNT_4096 4096U

#define AUDIO_ENVELOPE

#define AUDIO_UNINIT ((audio_context_t *)NULL)

typedef struct audio_context audio_context_t;

/**
 * Initialize the audio controller
 */
int audio_init(audio_context_t **context, size_t audio_sample_count);
void audio_feed_samples_24bit(audio_context_t *context, uint32_t *samples);
void audio_envelope(audio_context_t *context);
void audio_apply_gain(audio_context_t *context, float gain);
void audio_fft(audio_context_t *context);
const float *audio_get_frequency_bins(audio_context_t *context);
size_t audio_get_frequency_bin_count(audio_context_t *context);
void audio_deinit(audio_context_t **context);

#endif