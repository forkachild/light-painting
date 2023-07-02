#include "components/fft.h"
#include "pico/stdlib.h"
#include <complex.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Ultra fast log base-2 of only 2^n numbers. For others, the
 * result/behavior is invalid/undefined.
 *
 * Since 2^n numbers will have only one '1' bit, we just need to shift
 * right until we find it, and that's log2N
 *
 * @param N The input. *MUST BE A POWER OF 2*
 * @return uint The log base-2 result
 */
static inline uint log2N(uint N) {
    uint value, n;

    // Keep shifting right until we find a '1' at the LSB
    // and that's when we know we hit the jackpot!
    for (value = 0, n = N; (n & 0b1) == 0; n >>= 1, value++)
        ;

    return value;
}

/**
 * @brief O(n) order reverse bits.
 *
 * @param N Value to be bit-reversed
 * @param bit_depth Number of bits to be reversed
 * @return uint Bit-reversed number
 */
static inline uint reverse_bits(uint N, uint bit_depth) {
    uint output, i;

    // Simple, left shift one, right shift the other, bleh!
    for (output = 0, i = 0; i < bit_depth; i++, N >>= 1)
        output = (output << 1) | (N & 0b1);

    return output;
}

void fill_reversed_indices(uint *reversed_indices, uint N) {
    uint bit_depth, i;

    // Number of bits required
    bit_depth = log2N(N);

    for (i = 0; i < N; i++)
        reversed_indices[i] = reverse_bits(i, bit_depth);
}

void fill_twiddles(float complex *twiddles, uint N) {
    float angle_per_sample;
    uint halfN, i;

    // Mr. Clean
    halfN = N / 2;

    // -2pi/N, constant, reducing the calculations
    angle_per_sample = -2.0f * (float)M_PI / N;

    // Cache the twiddle factors
    // Compromise some space for HUGE performance gain
    // Cache locality baby!
    for (i = 0; i < halfN; i++)
        twiddles[i] = cexp(angle_per_sample * i * I);
}

void fill_twiddles_d(double complex *twiddles, uint N) {
    double angle_per_sample;
    uint halfN, i;

    // Mr. Clean
    halfN = N / 2;

    // -2pi/N, constant, reducing the calculations
    angle_per_sample = -2.0 * M_PI / N;

    // Cache the twiddle factors
    // Compromise some space for HUGE performance gain
    // Cache locality baby!
    for (i = 0; i < halfN; i++)
        twiddles[i] = cexp(angle_per_sample * i * I);
}

void fft_rad2_dit(float complex *X, const float complex *twiddles,
                  const uint *reversed_indices, uint N) {
    uint halfN, set_count, ops_per_set, set, start, butterfly,
        butterfly_top_idx, butterfly_bottom_idx;
    float complex twiddle, butterfly_top, butterfly_bottom;

    // Don't mess with me
    if (X == NULL || reversed_indices == NULL || twiddles == NULL)
        return;

    // Mr. Clean
    halfN = N / 2;

    // Perform the stages
    // i is the number of sets to perform
    // eg For N = 8
    // i = 4,2,1
    for (set_count = halfN; set_count >= 1; set_count >>= 1) {
        // No of operations per set in this stage
        // ops_per_set = 1,2,4
        ops_per_set = halfN / set_count;

        // Loop over sets
        // j is the set #
        for (set = 0; set < set_count; set++) {
            // Start the butterflies
            start = set * ops_per_set * 2;

            // Loop over butterflies
            for (butterfly = 0; butterfly < ops_per_set; butterfly++) {
                butterfly_top_idx = reversed_indices[start + butterfly];
                butterfly_bottom_idx =
                    reversed_indices[start + butterfly + ops_per_set];

                // Determine the twiddle to pre-multiply the lower
                // half of the butterfly
                twiddle = twiddles[butterfly * set_count]; // Cache hit baby!
                butterfly_top = X[butterfly_top_idx];
                butterfly_bottom = twiddle * X[butterfly_bottom_idx];

                // Finally the butterfly
                X[butterfly_top_idx] = butterfly_top + butterfly_bottom;
                X[butterfly_bottom_idx] = butterfly_top - butterfly_bottom;
            }
        }
    }
}

void fft_rad2_dit_d(double complex *X, const double complex *twiddles,
                    const uint *reversed_indices, uint N) {
    uint halfN, set_count, ops_per_set, set, start, butterfly,
        butterfly_top_idx, butterfly_bottom_idx;
    double complex twiddle, butterfly_top, butterfly_bottom;

    // Don't mess with me
    if (X == NULL || reversed_indices == NULL || twiddles == NULL)
        return;

    // Mr. Clean
    halfN = N / 2;

    // Perform the stages
    // i is the number of sets to perform
    // eg For N = 8
    // i = 4,2,1
    for (set_count = halfN; set_count >= 1; set_count >>= 1) {
        // No of operations per set in this stage
        // ops_per_set = 1,2,4
        ops_per_set = halfN / set_count;

        // Loop over sets
        // j is the set #
        for (set = 0; set < set_count; set++) {
            // Start the butterflies
            start = set * ops_per_set * 2;

            // Loop over butterflies
            for (butterfly = 0; butterfly < ops_per_set; butterfly++) {
                butterfly_top_idx = reversed_indices[start + butterfly];
                butterfly_bottom_idx =
                    reversed_indices[start + butterfly + ops_per_set];

                // Determine the twiddle to pre-multiply the lower
                // half of the butterfly
                twiddle = twiddles[butterfly * set_count]; // Cache hit baby!
                butterfly_top = X[butterfly_top_idx];
                butterfly_bottom = twiddle * X[butterfly_bottom_idx];

                // Finally the butterfly
                X[butterfly_top_idx] = butterfly_top + butterfly_bottom;
                X[butterfly_bottom_idx] = butterfly_top - butterfly_bottom;
            }
        }
    }
}

void fft_rad2_dif(float complex *X, const float complex *twiddles, uint N) {
    uint halfN, set_count, ops_per_set, set, start, butterfly,
        butterfly_top_idx, butterfly_bottom_idx;
    float complex twiddle, butterfly_top, butterfly_bottom;

    // Don't mess with me
    if (X == NULL || twiddles == NULL)
        return;

    // Mr. Clean!
    halfN = N / 2;

    // Perform the stages
    // i is the number of sets to perform
    // eg For N = 8
    // i = 1,2,4
    for (set_count = 1; set_count <= halfN; set_count <<= 1) {
        // No of operations per set in this stage
        // ops_per_set = 4,2,1
        ops_per_set = halfN / set_count;

        // Loop over sets
        for (set = 0; set < set_count; set++) {
            // Start the butterflies
            start = set * ops_per_set * 2;

            // Loop over butterflies
            for (butterfly = 0; butterfly < ops_per_set; butterfly++) {
                butterfly_top_idx = start + butterfly;
                butterfly_bottom_idx = start + butterfly + ops_per_set;

                // Determine the twiddle to pre-multiply the lower
                // half of the butterfly
                // Cache hit baby!
                twiddle = twiddles[butterfly * set_count];
                butterfly_top = X[butterfly_top_idx];
                butterfly_bottom = X[butterfly_bottom_idx];

                // Finally the butterfly
                X[butterfly_top_idx] = butterfly_top + butterfly_bottom;
                X[butterfly_bottom_idx] =
                    twiddle * (butterfly_top - butterfly_bottom);
            }
        }
    }
}

void fft_rad2_dif_d(double complex *X, const double complex *twiddles, uint N) {
    uint halfN, set_count, ops_per_set, set, start, butterfly,
        butterfly_top_idx, butterfly_bottom_idx;
    double complex twiddle, butterfly_top, butterfly_bottom;

    // Don't mess with me
    if (X == NULL || twiddles == NULL)
        return;

    // Mr. Clean
    halfN = N / 2;

    // Perform the stages
    // i is the number of sets to perform
    // eg For N = 8
    // i = 1,2,4
    for (set_count = 1; set_count <= halfN; set_count <<= 1) {
        // No of operations per set in this stage
        // ops_per_set = 4,2,1
        ops_per_set = halfN / set_count;

        // Loop over sets
        for (set = 0; set < set_count; set++) {
            // Start the butterflies
            start = set * ops_per_set * 2;

            // Loop over butterflies
            for (butterfly = 0; butterfly < ops_per_set; butterfly++) {
                butterfly_top_idx = start + butterfly;
                butterfly_bottom_idx = start + butterfly + ops_per_set;

                // Determine the twiddle to pre-multiply the lower
                // half of the butterfly
                // Cache hit baby!
                twiddle = twiddles[butterfly * set_count];
                butterfly_top = X[butterfly_top_idx];
                butterfly_bottom = X[butterfly_bottom_idx];

                // Finally the butterfly
                X[butterfly_top_idx] = butterfly_top + butterfly_bottom;
                X[butterfly_bottom_idx] =
                    twiddle * (butterfly_top - butterfly_bottom);
            }
        }
    }
}
