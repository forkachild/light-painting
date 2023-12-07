#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <complex.h>
#include <stdint.h>
#include <stdlib.h>

#define VISUALIZATION_UNINIT ((visualization_context_t *)NULL)

typedef struct visualization_context visualization_context_t;

int visualization_init(visualization_context_t **context, size_t pixel_count,
                       size_t audio_sample_count);
void visualization_feed_audio_samples(visualization_context_t *context,
                                      float *audio_sample_buffer);
size_t visualization_get_audio_sample_count(visualization_context_t *context);
size_t visualization_get_pixel_count(visualization_context_t *context);
float complex *visualization_get_audio_buffer(visualization_context_t *context);
const uint32_t *
visualization_get_pixel_buffer(visualization_context_t *context);
void visualization_deinit(visualization_context_t **context);

#endif