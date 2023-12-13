#include "audio.h"

#include <complex.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MIN_USABLE_FREQ 60
#define MAX_USABLE_FREQ 6000

static inline void generate_envelope(float *samples, size_t count) {
    float aDelta = 2.f * (float)M_PI / count;

    for (size_t i = 0; i < count; i++) {
        float angle = i * aDelta;
        angle -= (float)M_PI;
        samples[i] = .5f * (1.f + cosf(angle));
    }
}

static inline float *get_source_buffer(audio_t *self) {
    return self->sample_buffers[self->sample_cursor];
}

static inline size_t get_next_cursor(audio_t *self) {
    return (self->sample_cursor + 1) % SAMPLE_BUFFER_COUNT;
}

static inline float *get_dest_buffer(audio_t *self) {
    return self->sample_buffers[get_next_cursor(self)];
}

static inline void flip_buffers(audio_t *self) {
    self->sample_cursor = get_next_cursor(self);
}

int audio_init(audio_t *self, size_t sample_count, size_t sample_rate,
               float fft_smooth) {
    float *sample_buffer;
    float *envelope;
    float complex *fft_buffer;

    if ((sample_buffer = malloc(SAMPLE_BUFFER_COUNT * sample_count *
                                sizeof(float))) == NULL)
        return -1;

    if ((envelope = malloc(sample_count * sizeof(float))) == NULL) {
        free(sample_buffer);
        return -1;
    }

    if ((fft_buffer = malloc(sample_count * sizeof(float complex))) == NULL) {
        free(sample_buffer);
        free(envelope);
        return -1;
    }

    if (!fft_init(&self->fft, sample_count)) {
        free(sample_buffer);
        free(envelope);
        free(fft_buffer);
        return -1;
    }

    generate_envelope(envelope, sample_count);

    self->sample_rate = sample_rate;
    self->fft_smooth = fft_smooth;
    self->sample_count = sample_count;
    self->sample_cursor = 0;
    self->envelope = envelope;
    self->fft_buffer = fft_buffer;

    for (size_t i = 0; i < SAMPLE_BUFFER_COUNT; i++)
        self->sample_buffers[i] = &sample_buffer[i * sample_count];

    return 1;
}

void audio_feed_i2s_stereo_24bit(audio_t *self, const int32_t *samples,
                                 int32_t peak) {
    float peak_f = (float)peak;
    float scale_factor = 1.f / (2.f * peak_f);

    float *sample_buffer = get_source_buffer(self);

    for (size_t i = 0; i < self->sample_count; i++) {
        size_t index_l = 2 * i;
        size_t index_r = index_l + 1;

        //  Starts here     Ends here
        //  |                      |
        // x000000000000000000000000xxxxxxx
        // This is a 24-bit
        // One left shift aligns the MSB. Then it right shifted to align to
        // 24-bits.
        int32_t sample_l = (samples[index_l] << 1) >> 8;
        int32_t sample_r = (samples[index_r] << 1) >> 8;

        int32_t sample = sample_l + sample_r;

        // Scale and put
        sample_buffer[i] = (float)sample * scale_factor;
    }
}

void audio_scale_rms(audio_t *self) {
    float rms = 0;

    float *source_buffer = get_source_buffer(self);
    float *dest_buffer = get_dest_buffer(self);

    for (size_t i = 0; i < self->sample_count; i++) {
        float sample = source_buffer[i];
        rms += sample * sample;
    }

    rms = sqrtf(rms);
    rms /= self->sample_count;

    float rms_lb = .8f * rms;

    for (size_t i = 0; i < self->sample_count; i++) {
        float sample = source_buffer[i];

        if (fabsf(sample) < rms_lb)
            sample = 0.f;
        else
            sample /= rms;

        dest_buffer[i] = sample;
    }

    flip_buffers(self);
}

void audio_envelope(audio_t *self) {
    float *source_buffer = get_source_buffer(self);
    float *dest_buffer = get_dest_buffer(self);

    for (size_t i = 0; i < self->sample_count; i++)
        dest_buffer[i] = source_buffer[i] * self->envelope[i];

    flip_buffers(self);
}

void audio_multiply(audio_t *self, float value) {
    float *source_buffer = get_source_buffer(self);
    float *dest_buffer = get_dest_buffer(self);

    for (size_t i = 0; i < self->sample_count; i++)
        dest_buffer[i] = source_buffer[i] * value;

    flip_buffers(self);
}

void audio_square(audio_t *self) {
    float *source_buffer = get_source_buffer(self);
    float *dest_buffer = get_dest_buffer(self);

    for (size_t i = 0; i < self->sample_count; i++)
        dest_buffer[i] = source_buffer[i] * source_buffer[i];

    flip_buffers(self);
}

void audio_square_signed(audio_t *self) {
    float *source_buffer = get_source_buffer(self);
    float *dest_buffer = get_dest_buffer(self);

    for (size_t i = 0; i < self->sample_count; i++)
        dest_buffer[i] = source_buffer[i] * fabsf(source_buffer[i]);

    flip_buffers(self);
}

void audio_clip_above(audio_t *self, float amplitude) {
    float *source_buffer = get_source_buffer(self);
    float *dest_buffer = get_dest_buffer(self);

    for (size_t i = 0; i < self->sample_count; i++) {
        float sample = source_buffer[i];

        if (sample > amplitude)
            sample = 0.f;

        dest_buffer[i] = sample;
    }

    flip_buffers(self);
}

void audio_clip_below(audio_t *self, float amplitude) {
    float *source_buffer = get_source_buffer(self);
    float *dest_buffer = get_dest_buffer(self);

    for (size_t i = 0; i < self->sample_count; i++) {
        float sample = source_buffer[i];

        if (sample < amplitude)
            sample = 0.f;

        dest_buffer[i] = sample;
    }

    flip_buffers(self);
}

void audio_clip(audio_t *self) {
    float *source_buffer = get_source_buffer(self);
    float *dest_buffer = get_dest_buffer(self);

    for (size_t i = 0; i < self->sample_count; i++) {
        float sample = source_buffer[i];

        if (sample > 1.f)
            sample = 1.f;
        else if (sample < 0.f)
            sample = 0.f;

        dest_buffer[i] = sample;
    }

    flip_buffers(self);
}

void audio_normalize(audio_t *self) {
    float *source_buffer = get_source_buffer(self);
    float *dest_buffer = get_dest_buffer(self);

    float max = 0.f;

    for (size_t i = 0; i < self->sample_count; i++) {
        float sample = fabsf(source_buffer[i]);

        if (sample > max)
            max = sample;
    }

    for (size_t i = 0; i < self->sample_count; i++)
        dest_buffer[i] = source_buffer[i] / max;

    flip_buffers(self);
}

void audio_smooth_horizontal(audio_t *self, float factor) {
    if (factor == 0.f)
        return;

    float *source_buffer = get_source_buffer(self);

    for (size_t i = 1; i < self->sample_count; i++)
        source_buffer[i] = (factor * source_buffer[i - 1]) +
                           ((1.f - factor) * source_buffer[i]);
}

void audio_smooth_vertical(audio_t *self, float factor) {
    if (factor == 0.f)
        return;

    float *source_buffer = get_source_buffer(self);
    float *dest_buffer = get_dest_buffer(self);

    for (size_t i = 0; i < self->sample_count; i++)
        dest_buffer[i] =
            (factor * dest_buffer[i]) + ((1.f - factor) * source_buffer[i]);

    flip_buffers(self);
}

void audio_fft(audio_t *self) {
    float *source_buffer = get_source_buffer(self);
    float *dest_buffer = get_dest_buffer(self);

    for (size_t i = 0; i < self->sample_count; i++)
        // Implicit float to float complex cast
        self->fft_buffer[i] = source_buffer[i];

    fft_rad2_dit(&self->fft, self->fft_buffer, self->sample_count);

    // Determine maximum value for normalization
    for (size_t i = 0; i < self->sample_count / 2; i++)
        dest_buffer[i] = cabsf(self->fft_buffer[i]);

    flip_buffers(self);
}

void audio_fft_normalize(audio_t *self) {
    float *source_buffer = get_source_buffer(self);
    float *dest_buffer = get_dest_buffer(self);

    float max = 0.f;

    for (size_t i = 1; i < self->sample_count / 2; i++) {
        float sample = source_buffer[i];

        if (sample > max)
            max = sample;
    }

    for (size_t i = 0; i < self->sample_count; i++)
        dest_buffer[i] = source_buffer[i] / max;

    flip_buffers(self);
}

const float *audio_fft_spectra(audio_t *self) {
    size_t index = MIN_USABLE_FREQ * self->sample_count / self->sample_rate;
    return &get_source_buffer(self)[index];
}

size_t audio_fft_freq_spectra_count(audio_t *self) {
    return ((MAX_USABLE_FREQ - MIN_USABLE_FREQ) * self->sample_count /
            self->sample_rate);
}

void audio_deinit(audio_t *self) {
    free(self->sample_buffers[0]);
    free(self->envelope);
    free(self->fft_buffer);
    fft_deinit(&self->fft);
}
