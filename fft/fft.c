#include <complex.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "fft.h"
#include "pico/stdlib.h"

/**
 * @brief Ultra fast log base-2 of a number when it is power of 2
 *
 * @param N The input. MUST BE A POWER OF 2
 * @return uint The log base-2 result
 */
static inline uint log2N(uint N) {
    // Ultra cheap log2N, the number of skips to cover
    // 2's power numbers will only have a single '1' bit somewhere
    // 8 = 0b00001000, we have to shift 3 times to bring '1' to the LSB
    uint value, n;

    for (value = 0, n = N; (n & 0b1) == 0; n >>= 1, value++)
        ;

    return value;
}

static inline uint reverse_bits(uint input, uint bit_depth) {
    uint output, i;

    for (output = 0, i = 0; i < bit_depth; i++, input >>= 1)
        output = (output << 1) | (input & 0b1);

    return output;
}

uint *cache_reversed_indices(uint N) {
    uint *indices;
    uint bit_depth, i;

    indices = malloc(N * sizeof(uint));
    bit_depth = log2N(N);

    for (i = 0; i < N; i++)
        indices[i] = reverse_bits(i, bit_depth);

    return indices;
}

ComplexType *cache_twiddles(uint N) {
    RealType angle_per_sample, angle;
    uint halfN, k, sample;
    ComplexType *twiddles;

    halfN = N / 2;
    twiddles = malloc(halfN * sizeof(ComplexType));

    angle_per_sample = -2.0 * M_PI / N;

    // Cache the twiddle factors
    // Compromise some space for HUGE performance gain
    // Cache locality baby!
    for (sample = 0; sample < halfN; k++) {
        angle = angle_per_sample * sample;
        twiddles[k] = cexp(angle * I);
    }

    return twiddles;
}

void fft_dit_radix2(ComplexType *X, uint N, uint *reversed_indices,
                    ComplexType *twiddles) {
    uint halfN, set_count, ops, set, start, butterfly, butterfly_top_idx,
        butterfly_bottom_idx, twiddle_idx;
    ComplexType twiddle, butterfly_top, butterfly_bottom;

    if (X == NULL || reversed_indices == NULL || twiddles == NULL)
        return;

    halfN = N / 2;

    // Perform the stages
    // i is the number of sets to perform
    // eg For N = 8
    // i = 4,2,1
    for (set_count = halfN; set_count >= 1; set_count >>= 1) {
        // No of operations per set in this stage
        // ops = 1,2,4
        ops = halfN / set_count;

        // Loop over sets
        // j is the set #
        for (set = 0; set < set_count; set++) {
            // Start the butterflies
            start = set * ops * 2;

            // Loop over butterflies
            for (butterfly = 0; butterfly < ops; butterfly++) {
                butterfly_top_idx = reversed_indices[start + butterfly];
                butterfly_bottom_idx =
                    reversed_indices[start + butterfly + ops];
                twiddle_idx = butterfly * set_count;

                // Determine the twiddle to pre-multiply the lower
                // half of the butterfly
                twiddle = twiddles[twiddle_idx]; // Cache hit baby!
                butterfly_top = X[butterfly_top_idx];
                butterfly_bottom = twiddle * X[butterfly_bottom_idx];

                // Finally the butterfly
                X[butterfly_top_idx] = butterfly_top + butterfly_bottom;
                X[butterfly_bottom_idx] = butterfly_top - butterfly_bottom;
            }
        }
    }
}
