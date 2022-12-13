#pragma once

#include <complex.h>

#include "pico/types.h"

#ifdef DOUBLE_PRECISION
typedef double RealType;
typedef double complex ComplexType;
#else
typedef float RealType;
typedef float complex ComplexType;
#endif

/**
 * @brief Returns a pre-computed array of reversed indices
 *
 * @param N The number of samples. MUST BE A POWER OF 2
 * @return uint* The array of {index => bit-reversed index, ...} (size = N)
 */
uint *cache_reversed_indices(uint N);

/**
 * @brief Returns a pre-computed array of roots of unity used in FFT
 *
 * @param N The number of samples. MUST BE A POWER OF 2
 * @return ComplexType* The array of roots of unity (size = N)
 */
ComplexType *cache_twiddles(uint N);

/**
 * @brief Fast and efficient Decimation-in-Time Radix-2 In-Place Fast Fourier
 * Transform
 *
 * @param X Pointer to input samples. To be overwritten by (bit-reversed order)
 * frequency bins
 * @param N The number of samples. MUST BE A POWER OF 2
 * @param reversed_indices Pointer to pre-computed array of reversed indices
 * @param twiddles Pointer to pre-computed array of roots of unity
 */
void fft_dit_radix2(ComplexType *X, uint N, uint *reversed_indices,
                    ComplexType *twiddles);