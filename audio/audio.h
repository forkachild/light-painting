#ifndef AUDIO_H
#define AUDIO_H

#include "fft.h"
#include <pico/types.h>

#define SAMPLE_BUFFER_COUNT 2

typedef struct {
    size_t sample_rate;
    size_t sample_count;
    float fft_smooth;
    float *sample_buffers[SAMPLE_BUFFER_COUNT];
    size_t sample_cursor;
    float *envelope;
    float complex *fft_buffer;
    fft_t fft;
} audio_t;

int audio_init(audio_t *self, size_t sample_count, size_t sample_rate,
               float fft_smooth);

// Feed buffer(s)
void audio_feed_i2s_stereo_24bit(audio_t *self, const int32_t *samples,
                                 int32_t peak);

// Access buffers
// const float *audio_sample_buffer(audio_t *self);
// size_t audio_sample_count(audio_t *self);

// Effects/Filters/Operations
void audio_scale_rms(audio_t *self);
void audio_envelope(audio_t *self);
void audio_multiply(audio_t *self, float gain);
void audio_square(audio_t *self);
void audio_square_signed(audio_t *self);
void audio_clip_above(audio_t *self, float amplitude);
void audio_clip_below(audio_t *self, float amplitude);
void audio_clip(audio_t *self);
void audio_normalize(audio_t *self);
void audio_smooth_horizontal(audio_t *self, float factor);
void audio_smooth_vertical(audio_t *self, float factor);

// FFT
void audio_fft(audio_t *self);
void audio_fft_normalize(audio_t *self);
const float *audio_fft_spectra(audio_t *self);
size_t audio_fft_freq_spectra_count(audio_t *self);

// If you need documentation for self, give up. The world needs you in a
// different way.
void audio_deinit(audio_t *self);

#endif
