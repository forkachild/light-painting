#pragma once

#include "pico/types.h"
#include <complex.h>

void fill_reversed_indices(uint *reversed_indices, uint N);

void fill_twiddles(float complex *twiddles, uint N);

void fill_twiddles_d(double complex *twiddles, uint N);

void fft_dit_rad2(float complex *X, const float complex *twiddles,
                  const uint *reversed_indices, uint N);

void fft_dit_rad2_d(double complex *X, const double complex *twiddles,
                    const uint *reversed_indices, uint N);

void fft_dif_rad2(float complex *X, const float complex *twiddles, uint N);