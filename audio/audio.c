#include "audio.h"
#include "fft.h"
#include <complex.h>
#include <math.h>
#include <stdlib.h>

#define AMPLITUDE_24BIT ((uint32_t)0x00FFFFFF)

struct audio_context {
    size_t audio_sample_count;
    float complex *audio_sample_buffer;
    float *frequency_bins;
#ifdef AUDIO_ENVELOPE
    float *envelope;
#endif
    fft_context_t *fft;
};

#ifdef AUDIO_ENVELOPE
static inline void generate_envelope(float *samples, size_t count) {
    float aDelta = (float)M_PI / count;

    for (size_t i = 0; i < count; i++)
        samples[i] = sinf(i * aDelta);
}
#endif

int audio_init(audio_context_t **context, size_t audio_sample_count) {
    float complex *audio_sample_buffer;
    float *frequency_bins;
#ifdef AUDIO_ENVELOPE
    float *envelope = NULL;
#endif
    fft_context_t *fft;

    if (*context != AUDIO_UNINIT)
        return -1;

    audio_context_t *c = (audio_context_t *)malloc(sizeof(audio_context_t));

    if (c == NULL)
        return -1;

    audio_sample_buffer =
        (float complex *)malloc(audio_sample_count * sizeof(float complex));

    if (audio_sample_buffer == NULL) {
        free(c);
        return -1;
    }

    frequency_bins = (float *)malloc((audio_sample_count / 2) * sizeof(float));

    if (frequency_bins == NULL) {
        free(c);
        free(audio_sample_buffer);
        return -1;
    }

#ifdef AUDIO_ENVELOPE
    envelope = (float *)malloc(audio_sample_count * sizeof(float));

    if (envelope == NULL) {
        free(c);
        free(audio_sample_buffer);
        free(frequency_bins);
        return -1;
    }

    generate_envelope(envelope, audio_sample_count);
#endif

    if (!fft_init(&fft, audio_sample_count)) {
        free(c);
        free(audio_sample_buffer);
        free(frequency_bins);
#ifdef AUDIO_ENVELOPE
        free(envelope);
#endif
        return -1;
    }

    c->audio_sample_count = audio_sample_count;
    c->audio_sample_buffer = audio_sample_buffer;
    c->frequency_bins = frequency_bins;
#ifdef AUDIO_ENVELOPE
    c->envelope = envelope;
#endif
    c->fft = fft;

    *context = c;

    return 1;
}

void audio_feed_samples_24bit(audio_context_t *context, uint32_t *samples) {
    for (size_t i = 0; i < context->audio_sample_count; i++)
        context->audio_sample_buffer[i] = (float)samples[i] / AMPLITUDE_24BIT;
}

#ifdef AUDIO_ENVELOPE
void audio_envelope(audio_context_t *context) {
    for (size_t i = 0; i < context->audio_sample_count; i++)
        context->audio_sample_buffer[i] *= context->envelope[i];
}
#endif

void audio_apply_gain(audio_context_t *context, float gain) {
    for (size_t i = 0; i < context->audio_sample_count; i++)
        context->audio_sample_buffer[i] *= gain;
}

void audio_fft(audio_context_t *context) {
    fft_rad2_dif(context->fft, context->audio_sample_buffer,
                 context->frequency_bins);
}

const float *audio_get_frequency_bins(audio_context_t *context) {
    return context->frequency_bins;
}

size_t audio_get_frequency_bin_count(audio_context_t *context) {
    return context->audio_sample_count / 2;
}

void audio_deinit(audio_context_t **context) {
    audio_context_t *c = *context;

    if (c == NULL)
        return;

    free(c->audio_sample_buffer);
    free(c->frequency_bins);
    fft_deinit(&c->fft);
    free(c);

    *context = AUDIO_UNINIT;
}