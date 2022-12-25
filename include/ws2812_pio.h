#pragma once

#include "hardware/pio.h"
#include "pico/types.h"
#include "swapchain.h"
#include "types.h"

Result ws2812_init(uint count, uint pin);

bool ws2812_is_init();

int ws2812_get_count();

void ws2812_start_transmission();

void ws2812_stop_transmission();

Swapchain *ws2812_get_swapchain();

Result ws2812_deinit();