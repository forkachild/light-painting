#include "audio.h"

#include <complex.h>
#include <math.h>
#include <stdlib.h>

static inline void generate_envelope(float *samples, size_t count) {
    float aDelta = (float)M_PI / count;

    for (size_t i = 0; i < count; i++)
        samples[i] = sinf(i * aDelta);
}

int audio_init(audio_t *this, size_t sample_count) {
    float *sample_buffer;
    float *envelope;
    float *frequency_bins;
    float complex *fft_buffer;

    sample_buffer = (float *)malloc(sample_count * sizeof(float));
    if (sample_buffer == NULL)
        return -1;

    envelope = (float *)malloc(sample_count * sizeof(float));
    if (envelope == NULL) {
        free(sample_buffer);
        return -1;
    }

    frequency_bins = (float *)malloc((sample_count / 2) * sizeof(float));
    if (frequency_bins == NULL) {
        free(sample_buffer);
        free(envelope);
        return -1;
    }

    fft_buffer = (float complex *)malloc(sample_count * sizeof(float complex));
    if (fft_buffer == NULL) {
        free(sample_buffer);
        free(envelope);
        free(frequency_bins);
        return -1;
    }

    if (!fft_init(&this->fft, sample_count)) {
        free(sample_buffer);
        free(envelope);
        free(frequency_bins);
        free(fft_buffer);
        return -1;
    }

    generate_envelope(envelope, sample_count);

    this->sample_count = sample_count;
    this->sample_buffer = sample_buffer;
    this->envelope = envelope;
    this->frequency_bins = frequency_bins;
    this->fft_buffer = fft_buffer;

    return 1;
}

void audio_feed_i2s_stereo(audio_t *this, const int32_t *samples,
                           int32_t peak) {
    float peak_f = (float)peak;
    float factor = 1.f / (2 * peak_f);

    for (size_t i = 0; i < this->sample_count; i++) {
        size_t index_l = 2 * i;
        size_t index_r = index_l + 1;

        int32_t sample_l = (samples[index_l] << 1) >> 8;
        int32_t sample_r = (samples[index_r] << 1) >> 8;

        int32_t sample = sample_l + sample_r;

        // Scale and put
        this->sample_buffer[i] = (float)sample * factor;
    }
}

void audio_envelope(audio_t *this) {
    for (size_t i = 0; i < this->sample_count; i++)
        this->sample_buffer[i] *= this->envelope[i];
}

void audio_gain(audio_t *this, float gain) {
    for (size_t i = 0; i < this->sample_count; i++)
        this->sample_buffer[i] *= gain;
}

void audio_fft(audio_t *this) {
    for (size_t i = 0; i < this->sample_count; i++)
        // Implicit float to float complex cast
        this->fft_buffer[i] = this->sample_buffer[i];

    fft_rad2_dif(&this->fft, this->fft_buffer, this->frequency_bins);
}

const float *audio_get_frequency_bins(audio_t *this) {
    return this->frequency_bins;
}

size_t audio_get_frequency_bin_count(audio_t *this) {
    return this->sample_count / 2;
}

void audio_deinit(audio_t *this) {
    free(this->sample_buffer);
    free(this->envelope);
    free(this->frequency_bins);
    free(this->fft_buffer);
    fft_deinit(&this->fft);
}
