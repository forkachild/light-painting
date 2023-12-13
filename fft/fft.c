#include "fft.h"

#include <complex.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Ultra fast log base-2 of only 2^n numbers.
 *
 * Since 2^n numbers have only one '1' bit, we just need to shift
 * right until we find it
 *
 * @param N The input.
 * @return The log base 2 result, -1 if not a power of two
 */
static inline int log2N(unsigned int N) {
    int value, n;

    // Keep shifting right until we find a '1' at the LSB
    // and that's when we know we hit the jackpot!
    for (value = 0, n = N; (n & 0b1) == 0; n >>= 1, value++)
        ;

    if (n == 1)
        return value;

    return -1;
}

/**
 * @brief O(n) order reverse bits.
 *
 * @param N Value to be bit-reversed
 * @param bit_depth Number of bits to be reversed
 * @return unsigned int Bit-reversed number
 */
static inline unsigned int reverse_bits(unsigned int N,
                                        unsigned int bit_depth) {
    unsigned int output, i;

    // Simple, left shift one, right shift the other, bleh!
    for (output = 0, i = 0; i < bit_depth; i++, N >>= 1)
        output = (output << 1) | (N & 0b1);

    return output;
}

static int fill_bit_reverse_map(unsigned int *bit_reverse_map, unsigned int N) {
    int bit_depth, i;

    if ((bit_depth = log2N(N)) == -1)
        return -1;

    for (i = 0; i < (int)N; i++)
        bit_reverse_map[i] = reverse_bits(i, bit_depth);

    return 1;
}

static void fill_twiddles(float complex *twiddles, unsigned int N) {
    float angle_per_sample;
    unsigned int i;

    // -2pi/N, constant, reducing the calculations
    angle_per_sample = -2.0f * (float)M_PI / N;

    // Cache the twiddle factors
    // Compromise some space for HUGE performance gain
    // Cache locality baby!
    for (i = 0; i < N; i++)
        twiddles[i] = cexp(angle_per_sample * i * I);
}

static void fill_twiddles_d(double complex *twiddles, unsigned int N) {
    double angle_per_sample;
    unsigned int i;

    // -2pi/N, constant, reducing the calculations
    angle_per_sample = -2.0 * M_PI / N;

    // Cache the twiddle factors
    // Compromise some space for HUGE performance gain
    // Cache locality baby!
    for (i = 0; i < N; i++)
        twiddles[i] = cexp(angle_per_sample * i * I);
}

static void bit_reverse(float complex *samples, unsigned int *bit_reverse_map,
                        unsigned int count) {

    for (unsigned int i = 0; i < count; i++) {
        unsigned int rev_i = bit_reverse_map[i];

        // Prevent double swapping and same-place swaps
        if (rev_i > i) {
            float complex temp = samples[i];
            samples[i] = samples[rev_i];
            samples[rev_i] = temp;
        }
    }
}

static void bit_reverse_d(double complex *samples,
                          unsigned int *bit_reverse_map, unsigned int count) {

    for (unsigned int i = 0; i < count; i++) {
        unsigned int rev_i = bit_reverse_map[i];

        // Prevent double swapping and same-place swaps
        if (rev_i > i) {
            double complex temp = samples[i];
            samples[i] = samples[rev_i];
            samples[rev_i] = temp;
        }
    }
}

int fft_init(fft_t *this, unsigned int count) {
    unsigned int *bit_reverse_map;
    float complex *twiddles;

    bit_reverse_map = (unsigned int *)malloc(count * sizeof(unsigned int));

    if (bit_reverse_map == NULL)
        return -1;

    twiddles = (float complex *)malloc((count / 2) * sizeof(float complex));

    if (twiddles == NULL) {
        free(bit_reverse_map);
        return -1;
    }

    if (!fill_bit_reverse_map(bit_reverse_map, count)) {
        free(bit_reverse_map);
        free(twiddles);
        return -1;
    }

    fill_twiddles(twiddles, count / 2);

    this->count = count;
    this->bit_reverse_map = bit_reverse_map;
    this->twiddles = twiddles;

    return 1;
}

void fft_rad2_dit(fft_t *this, float complex *samples,
                  unsigned int sample_count) {
    unsigned int half_sample_count, dft_count, butterfly_count, dft, dft_offset,
        butterfly, top_idx, bottom_idx, twiddle_idx;
    float complex top, bottom;

    // Don't mess with me
    if (samples == NULL || this->count != sample_count)
        return;

    // Reverse the array in-place
    bit_reverse(samples, this->bit_reverse_map, this->count);

    // Mr. Clean
    half_sample_count = this->count / 2;

    // Loop over each stage
    // dft_count is the number of DFTs to perform in the stage
    // eg For N = 8
    // dft_count = 4,2,1
    for (dft_count = half_sample_count; dft_count >= 1; dft_count >>= 1) {
        // No of butterflies per DFT in this stage
        // butterfly_count = 1,2,4
        butterfly_count = half_sample_count / dft_count;

        // Loop over each DFT in this stage
        for (dft = 0; dft < dft_count; dft++) {
            // Start the butterflies
            dft_offset = dft * butterfly_count * 2;

            // Loop over butterflies in this DFT
            for (butterfly = 0; butterfly < butterfly_count; butterfly++) {
                top_idx = dft_offset + butterfly;
                bottom_idx = top_idx + butterfly_count;
                twiddle_idx = butterfly * dft_count;

                // Prepare butterfly wings
                top = samples[top_idx];
                bottom = this->twiddles[twiddle_idx] * samples[bottom_idx];

                // Finally the butterfly
                samples[top_idx] = top + bottom;
                samples[bottom_idx] = top - bottom;
            }
        }
    }
}

void fft_rad2_dif(fft_t *this, float complex *samples,
                  unsigned int sample_count) {
    unsigned int half_sample_count, dft_count, butterfly_count, dft, dft_offset,
        butterfly, top_idx, bottom_idx, twiddle_idx;
    float complex top, bottom;

    // Don't mess with me
    if (samples == NULL || this->count != sample_count)
        return;

    // Mr. Clean
    half_sample_count = this->count / 2;

    // Loop over each stage
    // dft_count is the number of DFTs to perform in the stage
    // eg For N = 8
    // dft_count = 4,2,1
    for (dft_count = 1; dft_count <= half_sample_count; dft_count <<= 1) {
        // No of butterflies per DFT in this stage
        // butterfly_count = 1,2,4
        butterfly_count = half_sample_count / dft_count;

        // Loop over each DFT in this stage
        for (dft = 0; dft < dft_count; dft++) {
            // Start the butterflies
            dft_offset = dft * butterfly_count * 2;

            // Loop over butterflies in this DFT
            for (butterfly = 0; butterfly < butterfly_count; butterfly++) {
                top_idx = dft_offset + butterfly;
                bottom_idx = top_idx + butterfly_count;
                twiddle_idx = butterfly * dft_count;

                // Prepare butterfly wings
                top = samples[top_idx];
                bottom = samples[bottom_idx];

                // Finally the butterfly
                samples[top_idx] = top + bottom;
                samples[bottom_idx] =
                    this->twiddles[twiddle_idx] * (top - bottom);
            }
        }
    }

    bit_reverse(samples, this->bit_reverse_map, this->count);
}

void fft_deinit(fft_t *this) {
    free(this->twiddles);
    free(this->bit_reverse_map);
}

int fft_init_d(fft_d_t *this, unsigned int count) {
    unsigned int *bit_reverse_map;
    double complex *twiddles;

    bit_reverse_map = (unsigned int *)malloc(count * sizeof(unsigned int));
    if (bit_reverse_map == NULL)
        return -1;

    twiddles = (double complex *)malloc((count / 2) * sizeof(double complex));
    if (twiddles == NULL) {
        free(bit_reverse_map);
        return -1;
    }

    fill_bit_reverse_map(bit_reverse_map, count);
    fill_twiddles_d(twiddles, count / 2);

    this->count = count;
    this->bit_reverse_map = bit_reverse_map;
    this->twiddles = twiddles;

    return 1;
}

void fft_rad2_dit_d(fft_d_t *this, double complex *samples,
                    unsigned int sample_count) {
    unsigned int half_sample_count, dft_count, butterfly_count, dft, dft_offset,
        butterfly, top_idx, bottom_idx, twiddle_idx;
    double complex top, bottom;

    // Don't mess with me
    if (samples == NULL || this->count != sample_count)
        return;

    // Reverse the array in-place
    bit_reverse_d(samples, this->bit_reverse_map, this->count);

    // Mr. Clean
    half_sample_count = this->count / 2;

    // Loop over each stage
    // dft_count is the number of DFTs to perform in the stage
    // eg For N = 8
    // dft_count = 4,2,1
    for (dft_count = half_sample_count; dft_count >= 1; dft_count >>= 1) {
        // No of butterflies per DFT in this stage
        // butterfly_count = 1,2,4
        butterfly_count = half_sample_count / dft_count;

        // Loop over each DFT in this stage
        for (dft = 0; dft < dft_count; dft++) {
            // Start the butterflies
            dft_offset = dft * butterfly_count * 2;

            // Loop over butterflies in this DFT
            for (butterfly = 0; butterfly < butterfly_count; butterfly++) {
                top_idx = dft_offset + butterfly;
                bottom_idx = top_idx + butterfly_count;
                twiddle_idx = butterfly * dft_count;

                // Prepare butterfly wings
                top = samples[top_idx];
                bottom = this->twiddles[twiddle_idx] * samples[bottom_idx];

                // Finally the butterfly
                samples[top_idx] = top + bottom;
                samples[bottom_idx] = top - bottom;
            }
        }
    }
}

void fft_rad2_dif_d(fft_d_t *this, double complex *samples,
                    unsigned int sample_count) {
    unsigned int half_sample_count, dft_count, butterfly_count, dft, dft_offset,
        butterfly, top_idx, bottom_idx, twiddle_idx;
    float complex top, bottom;

    // Don't mess with me
    if (samples == NULL || this->count != sample_count)
        return;

    // Mr. Clean
    half_sample_count = this->count / 2;

    // Loop over each stage
    // dft_count is the number of DFTs to perform in the stage
    // eg For N = 8
    // dft_count = 4,2,1
    for (dft_count = 1; dft_count <= half_sample_count; dft_count <<= 1) {
        // No of butterflies per DFT in this stage
        // butterfly_count = 1,2,4
        butterfly_count = half_sample_count / dft_count;

        // Loop over each DFT in this stage
        for (dft = 0; dft < dft_count; dft++) {
            // Start the butterflies
            dft_offset = dft * butterfly_count * 2;

            // Loop over butterflies in this DFT
            for (butterfly = 0; butterfly < butterfly_count; butterfly++) {
                top_idx = dft_offset + butterfly;
                bottom_idx = top_idx + butterfly_count;
                twiddle_idx = butterfly * dft_count;

                // Prepare butterfly wings
                top = samples[top_idx];
                bottom = samples[bottom_idx];

                // Finally the butterfly
                samples[top_idx] = top + bottom;
                samples[bottom_idx] =
                    this->twiddles[twiddle_idx] * (top - bottom);
            }
        }
    }

    bit_reverse_d(samples, this->bit_reverse_map, this->count);
}

void fft_deinit_d(fft_d_t *this) {
    free(this->twiddles);
    free(this->bit_reverse_map);
}