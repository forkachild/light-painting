#ifndef AUDIO_H
#define AUDIO_H

#include <complex.h>
#include <pico/types.h>

#define AUDIO_UNINIT ((audio_context_t *)NULL)

typedef struct audio_context audio_context_t;

int audio_init(audio_context_t **context, size_t audio_sample_count);
void audio_feed_inmp441(audio_context_t *context, const uint32_t *samples);

#ifdef AUDIO_ENVELOPE
void audio_envelope(audio_context_t *context);
#endif

void audio_gain(audio_context_t *context, float gain);
void audio_fft(audio_context_t *context);
const float *audio_get_frequency_bins(audio_context_t *context);
size_t audio_get_frequency_bin_count(audio_context_t *context);
void audio_deinit(audio_context_t **context);

#endif