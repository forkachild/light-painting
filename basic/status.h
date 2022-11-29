#pragma once

#include <stdbool.h>
#include <stdint.h>

#define STATUS_DELAY_FAST_MS 300
#define STATUS_DELAY_MEDIUM_MS 700
#define STATUS_DELAY_SLOW_MS 1500

void ws2812_status_init();
void ws2812_status_show(bool value);
void ws2812_status_blink_blocking(uint32_t times, uint32_t delay);
void ws2812_status_panic_blocking(uint32_t delay);
void ws2812_status_deinit();