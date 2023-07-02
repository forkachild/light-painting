#pragma once

#include "components/buffer.h"
#include "types.h"

Result ws2812_init(uint count, uint pin);

bool ws2812_is_init();

int ws2812_get_count();

void ws2812_start_transmission();

void ws2812_stop_transmission();

AsyncBuffer *ws2812_get_async_buffer();

Result ws2812_deinit();