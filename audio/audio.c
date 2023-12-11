#include "audio.h"

#include <complex.h>
#include <math.h>
#include <stdlib.h>

static inline void generate_envelope(float *samples, size_t count) {
    float aDelta = (float)M_PI / count;

    for (size_t i = 0; i < count; i++) {
        float angle = i * aDelta;
        angle -= (float)M_PI_2;
        samples[i] = 0.5f * (1.f + cosf(i * aDelta));
    }
}

int audio_init(audio_t *this, size_t sample_count) {
    float *sample_buffer;
    float *envelope;
    float complex *fft_buffer;

    if ((sample_buffer = malloc(sample_count * sizeof(float))) == NULL)
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

    if (!fft_init(&this->fft, sample_count)) {
        free(sample_buffer);
        free(envelope);
        free(fft_buffer);
        return -1;
    }

    generate_envelope(envelope, sample_count);

    this->sample_count = sample_count;
    this->sample_buffer = sample_buffer;
    this->envelope = envelope;
    this->fft_buffer = fft_buffer;

    return 1;
}

void audio_feed_i2s_stereo(audio_t *this, const int32_t *samples,
                           int32_t peak) {
    float peak_f = (float)peak;
    float factor = 1.f / (2.f * peak_f);

    for (size_t i = 0; i < this->sample_count; i++) {
        size_t index_l = 2 * i;
        size_t index_r = index_l + 1;

        //  Starts here     Ends here
        //  |                      |
        // x000000000000000000000000xxxxxxx
        // This is a 24-bit
        // One left shift involves the MSB for the signed right shift.
        int32_t sample_l = (samples[index_l] << 1) >> 8;
        int32_t sample_r = (samples[index_r] << 1) >> 8;

        int32_t sample = sample_l + sample_r;

        // Scale and put
        this->sample_buffer[i] = (float)sample * factor;
    }
}

const float *audio_sample_buffer(audio_t *this) { return this->sample_buffer; }

size_t audio_sample_count(audio_t *this) { return this->sample_count; }

void audio_envelope(audio_t *this) {
    for (size_t i = 0; i < this->sample_count; i++)
        this->sample_buffer[i] *= this->envelope[i];
}

void audio_multiply(audio_t *this, float value) {
    for (size_t i = 0; i < this->sample_count; i++)
        this->sample_buffer[i] *= value;
}

void audio_square(audio_t *this) {
    for (size_t i = 0; i < this->sample_count; i++)
        this->sample_buffer[i] *= this->sample_buffer[i];
}

void audio_square_signed(audio_t *this) {
    for (size_t i = 0; i < this->sample_count; i++)
        this->sample_buffer[i] *= fabsf(this->sample_buffer[i]);
}

void audio_clip(audio_t *this) {
    for (size_t i = 0; i < this->sample_count; i++) {
        float sample = this->sample_buffer[i];

        if (sample > 1.f)
            sample = 1.f;
        else if (sample < 0.f)
            sample = 0.f;

        this->sample_buffer[i] = sample;
    }
}

void audio_normalize(audio_t *this) {
    float max = 0.f;

    for (size_t i = 0; i < this->sample_count; i++) {
        float sample_abs = fabsf(this->sample_buffer[i]);

        if (sample_abs > max)
            max = sample_abs;
    }

    if (max == 0.f)
        return;

    for (size_t i = 0; i < this->sample_count; i++)
        this->sample_buffer[i] /= max;
}

void audio_smooth(audio_t *this, float factor) {
    if (factor == 0.f)
        return;

    for (size_t i = 1; i < this->sample_count; i++)
        this->sample_buffer[i] = ((1.f - factor) * this->sample_buffer[i]) +
                                 (factor * this->sample_buffer[i - 1]);
}

void audio_fft(audio_t *this) {
    for (size_t i = 0; i < this->sample_count; i++)
        // Implicit float to float complex cast
        this->fft_buffer[i] = this->sample_buffer[i];

    fft_rad2_dit(&this->fft, this->fft_buffer);

    for (size_t i = 0; i < this->sample_count / 2; i++)
        this->sample_buffer[i] = cabsf(this->fft_buffer[i]);
}

const float *audio_fft_freq_bins(audio_t *this) {
    return &this->sample_buffer[1];
}

size_t audio_fft_freq_bin_count(audio_t *this) {
    return (this->sample_count / 2) - 1;
}

void audio_deinit(audio_t *this) {
    free(this->sample_buffer);
    free(this->envelope);
    free(this->fft_buffer);
    fft_deinit(&this->fft);
}
