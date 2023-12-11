#ifndef AUDIO_H
#define AUDIO_H

#include "fft.h"
#include <pico/types.h>

typedef struct {
    size_t sample_count;
    float *sample_buffer;
    float *envelope;
    float complex *fft_buffer;
    float *frequency_bins;
    fft_t fft;
} audio_t;

int audio_init(audio_t *this, size_t sample_count);
void audio_feed_i2s_stereo(audio_t *context, const int32_t *samples,
                           int32_t peak);
void audio_envelope(audio_t *this);
void audio_gain(audio_t *this, float gain);
void audio_fft(audio_t *this);
const float *audio_get_frequency_bins(audio_t *this);
size_t audio_get_frequency_bin_count(audio_t *this);
void audio_deinit(audio_t *this);

#endif
