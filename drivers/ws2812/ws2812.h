#ifndef WS2812_PIO_H
#define WS2812_PIO_H

#include <pico/types.h>

int ws2812_init(uint count, uint pin);

bool ws2812_is_init();

int ws2812_get_count();

void ws2812_start_transmission();

void ws2812_stop_transmission();

void *ws2812_get_async_buffer();

void ws2812_deinit();

#endif