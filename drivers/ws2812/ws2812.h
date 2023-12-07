#ifndef WS2812_PIO_H
#define WS2812_PIO_H

#include <pico/types.h>

int ws2812_init(uint count, uint pin);

bool ws2812_is_init();

void ws2812_start_transmission();

void ws2812_stop_transmission();

size_t ws2812_get_pixel_count();

void *ws2812_get_pixel_buffer();

void ws2812_swap_buffers();

void ws2812_deinit();

#endif