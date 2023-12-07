#ifndef INMP441_PIO_H
#define INMP441_PIO_H

#include <pico/types.h>

#define MAX_AMPLITUDE 0x00FFFFFF

int inmp441_init(uint sample_count, uint sck_pin, uint ws_pin, uint data_pin);

void inmp441_start_sampling();

void inmp441_stop_sampling();

/**
 * Swap in a fresh buffer
 */
void inmp441_swap_buffers();

/**
 * Cleans and aligns the samples to 24-bits
 * Recommended to be called before
 */
void inmp441_normalize_audio_buffer();

/**
 * Get the buffer to read from
 */
const uint32_t *inmp441_get_audio_buffer();

void inmp441_deinit();

#endif
