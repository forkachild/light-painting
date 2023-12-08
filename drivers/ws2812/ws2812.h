#ifndef WS2812_PIO_H
#define WS2812_PIO_H

#include "swapchain.h"
#include <pico/types.h>

size_t ws2812_required_buffer_size(size_t led_count);

int ws2812_init(swapchain_t *swapchain, size_t count, uint pin);

size_t ws2812_get_pixel_count();

void ws2812_start_transmission();

void ws2812_stop_transmission();

void ws2812_deinit();

#endif