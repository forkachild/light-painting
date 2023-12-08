#ifndef INMP441_PIO_H
#define INMP441_PIO_H

#include "swapchain.h"
#include <pico/types.h>

size_t inmp441_required_buffer_size(size_t sample_count);

int inmp441_init(swapchain_context_t *swapchain, size_t sample_count,
                 uint sck_pin, uint ws_pin, uint data_pin);

size_t inmp441_sample_count();

void inmp441_start_sampling();

void inmp441_stop_sampling();

void inmp441_deinit();

#endif
