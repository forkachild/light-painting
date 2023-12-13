#ifndef FFT_H
#define FFT_H

#include <complex.h>
#include <stdint.h>

typedef struct {
    unsigned int *bit_reverse_map;
    float complex *twiddles;
    size_t count;
} fft_t;

typedef struct {
    unsigned int *bit_reverse_map;
    double complex *twiddles;
    size_t count;
} fft_d_t;

int fft_init(fft_t *this, size_t count);
void fft_rad2_dit(fft_t *this, float complex *samples,
                  unsigned int sample_count);
void fft_rad2_dif(fft_t *this, float complex *samples,
                  unsigned int sample_count);
void fft_deinit(fft_t *this);

int fft_init_d(fft_d_t *this, size_t count);
void fft_rad2_dit_d(fft_d_t *this, double complex *samples,
                    unsigned int sample_count);
void fft_rad2_dif_d(fft_d_t *this, double complex *samples,
                    unsigned int sample_count);
void fft_deinit_d(fft_d_t *this);

#endif