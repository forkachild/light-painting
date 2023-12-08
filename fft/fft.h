#ifndef FFT_H
#define FFT_H

#include <complex.h>
#include <stdint.h>

#define FFT_UNINIT ((fft_context_t *)NULL)
#define FFT_UNINIT_D ((fft_context_d_t *)NULL)

typedef struct fft_context fft_context_t;
typedef struct fft_context_d fft_context_d_t;

int fft_init(fft_context_t **context, unsigned int count);
int fft_init_d(fft_context_d_t **context, unsigned int count);
void fft_rad2_dit(fft_context_t *context, float complex *samples,
                  float *frequency_bins);
void fft_rad2_dit_d(fft_context_d_t *context, double complex *samples,
                    double *frequency_bins);
void fft_rad2_dif(fft_context_t *context, float complex *samples,
                  float *frequency_bins);
void fft_rad2_dif_d(fft_context_d_t *context, double complex *samples,
                    double *frequency_bins);
const float *fft_get_frequency_bins(fft_context_t *context);
void fft_deinit(fft_context_t **context);
void fft_deinit_d(fft_context_d_t **context);

#endif