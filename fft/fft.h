#pragma once

#include <complex.h>

#include "pico/types.h"

/**
 * @brief Returns a pre-computed array of reversed indices
 *
 * @param N The number of samples. MUST BE A POWER OF 2
 * @return uint* The array of {index => bit-reversed index, ...} (size = N)
 */
void fill_reversed_indices(uint *reversed_indices, uint N);

/**
 * @brief Returns a pre-computed float complex * array of roots of unity used in
 * FFT
 *
 * @param twiddles Pointer to a double complex * array
 * @param N The number of samples. MUST BE A POWER OF 2
 * @return ComplexType* The array of roots of unity (size = N)
 */
void fill_twiddles(float complex *twiddles, uint N);

/**
 * @brief Returns a pre-computed double complex * array of roots of unity used
 * in FFT
 *
 * @param twiddles Pointer to a double complex * array
 * @param N The number of samples. MUST BE A POWER OF 2
 * @return ComplexType* The array of roots of unity (size = N)
 */
void fill_twiddles_d(double complex *twiddles, uint N);

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
void fft_dit_rad2(float complex *X, const float complex *twiddles,
                  const uint *reversed_indices, uint N);

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
void fft_dit_rad2_d(double complex *X, const double complex *twiddles,
                    const uint *reversed_indices, uint N);

/**
 * @brief Fast and efficient Decimation-in-Frequency Radix-2 In-Place Fast
 * Fourier Transform
 *
 * @param X Pointer to input samples. To be overwritten by (bit-reversed order)
 * frequency bins
 * @param N The number of samples. MUST BE A POWER OF 2
 * @param reversed_indices Pointer to pre-computed array of reversed indices
 * @param twiddles Pointer to pre-computed array of roots of unity
 */
void fft_dif_rad2(float complex *X, const float complex *twiddles, uint N);

/**
 * @brief Fast and efficient Decimation-in-Frequency Radix-2 In-Place Fast
 * Fourier Transform
 *
 * @param X Pointer to input samples. To be overwritten by (bit-reversed order)
 * frequency bins
 * @param N The number of samples. MUST BE A POWER OF 2
 * @param reversed_indices Pointer to pre-computed array of reversed indices
 * @param twiddles Pointer to pre-computed array of roots of unity
 */
void fft_dif_rad2_d(double complex *X, const double complex *twiddles, uint N);