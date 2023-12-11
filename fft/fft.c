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

static void fill_bit_reverse_map(unsigned int *bit_reverse_map,
                                 unsigned int N) {
    unsigned int bit_depth, i;

    // Number of bits required
    bit_depth = log2N(N);

    for (i = 0; i < N; i++)
        bit_reverse_map[i] = reverse_bits(i, bit_depth);
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

    fill_bit_reverse_map(bit_reverse_map, count);
    fill_twiddles(twiddles, count / 2);

    this->count = count;
    this->bit_reverse_map = bit_reverse_map;
    this->twiddles = twiddles;

    return 1;
}

void fft_rad2_dit(fft_t *this, float complex *samples) {
    unsigned int halfN, set_count, ops_per_set, set, start, butterfly,
        butterfly_top_idx, butterfly_bottom_idx;
    float complex twiddle, butterfly_top, butterfly_bottom;

    // Don't mess with me
    if (samples == NULL)
        return;

    // Reverse the array in-place
    bit_reverse(samples, this->bit_reverse_map, this->count);

    // Mr. Clean
    halfN = this->count / 2;

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
                butterfly_top_idx = start + butterfly;
                butterfly_bottom_idx = start + butterfly + ops_per_set;

                // Determine the twiddle to pre-multiply the lower
                // half of the butterfly
                twiddle =
                    this->twiddles[butterfly * set_count]; // Cache hit baby!
                butterfly_top = samples[butterfly_top_idx];
                butterfly_bottom = twiddle * samples[butterfly_bottom_idx];

                // Finally the butterfly
                samples[butterfly_top_idx] = butterfly_top + butterfly_bottom;
                samples[butterfly_bottom_idx] =
                    butterfly_top - butterfly_bottom;
            }
        }
    }
}

void fft_rad2_dif(fft_t *this, float complex *samples) {
    unsigned int halfN, set_count, ops_per_set, set, start, butterfly,
        butterfly_top_idx, butterfly_bottom_idx;
    float complex twiddle, butterfly_top, butterfly_bottom;

    // Don't mess with me
    if (samples == NULL)
        return;

    // Mr. Clean!
    halfN = this->count / 2;

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
                twiddle = this->twiddles[butterfly * set_count];
                butterfly_top = samples[butterfly_top_idx];
                butterfly_bottom = samples[butterfly_bottom_idx];

                // Finally the butterfly
                samples[butterfly_top_idx] = butterfly_top + butterfly_bottom;
                samples[butterfly_bottom_idx] =
                    twiddle * (butterfly_top - butterfly_bottom);
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

void fft_rad2_dit_d(fft_d_t *this, double complex *samples) {
    unsigned int halfN, set_count, ops_per_set, set, start, butterfly,
        butterfly_top_idx, butterfly_bottom_idx;
    double complex twiddle, butterfly_top, butterfly_bottom;

    // Don't mess with me
    if (samples == NULL)
        return;

    bit_reverse_d(samples, this->bit_reverse_map, this->count);

    // Mr. Clean
    halfN = this->count / 2;

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
                butterfly_top_idx = this->bit_reverse_map[start + butterfly];
                butterfly_bottom_idx =
                    this->bit_reverse_map[start + butterfly + ops_per_set];

                // Determine the twiddle to pre-multiply the lower
                // half of the butterfly
                twiddle =
                    this->twiddles[butterfly * set_count]; // Cache hit baby!
                butterfly_top = samples[butterfly_top_idx];
                butterfly_bottom = twiddle * samples[butterfly_bottom_idx];

                // Finally the butterfly
                samples[butterfly_top_idx] = butterfly_top + butterfly_bottom;
                samples[butterfly_bottom_idx] =
                    butterfly_top - butterfly_bottom;
            }
        }
    }
}

void fft_rad2_dif_d(fft_d_t *this, double complex *samples) {
    unsigned int halfN, set_count, ops_per_set, set, start, butterfly,
        butterfly_top_idx, butterfly_bottom_idx;
    double complex twiddle, butterfly_top, butterfly_bottom;

    // Don't mess with me
    if (samples == NULL)
        return;

    // Mr. Clean
    halfN = this->count / 2;

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
                twiddle = this->twiddles[butterfly * set_count];
                butterfly_top = samples[butterfly_top_idx];
                butterfly_bottom = samples[butterfly_bottom_idx];

                // Finally the butterfly
                samples[butterfly_top_idx] = butterfly_top + butterfly_bottom;
                samples[butterfly_bottom_idx] =
                    twiddle * (butterfly_top - butterfly_bottom);
            }
        }
    }

    bit_reverse_d(samples, this->bit_reverse_map, this->count);
}

void fft_deinit_d(fft_d_t *this) {
    free(this->twiddles);
    free(this->bit_reverse_map);
}