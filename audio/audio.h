#ifndef AUDIO_H
#define AUDIO_H

#include "fft.h"
#include <pico/types.h>

typedef struct {
    size_t sample_count;
    float *sample_buffer;
    float *envelope;
    float complex *fft_buffer;
    fft_t fft;
} audio_t;

int audio_init(audio_t *this, size_t sample_count);

// Feed buffer(s)
void audio_feed_i2s_stereo(audio_t *context, const int32_t *samples,
                           int32_t peak);

// Access buffers
const float *audio_sample_buffer(audio_t *this);
size_t audio_sample_count(audio_t *this);

// Effects/Filters/Operations
void audio_envelope(audio_t *this);
void audio_multiply(audio_t *this, float gain);
void audio_square(audio_t *this);
void audio_square_signed(audio_t *this);
void audio_clip(audio_t *this);
void audio_normalize(audio_t *this);
void audio_smooth(audio_t *this, float factor);
void audio_fft(audio_t *this);
const float *audio_fft_freq_bins(audio_t *this);
size_t audio_fft_freq_bin_count(audio_t *this);

// If you need documentation for this, give up. The world needs you in a
// different way.
void audio_deinit(audio_t *this);

#endif
