#pragma once

#include <stdlib.h>
#include "pico/stdlib.h"
#include <stdint.h>

typedef struct WS2812Driver WS2812Driver;

void ws2812_driver_init(WS2812Driver **ppDriver, size_t ledCount);
void ws2812_driver_submit_argb_buffer_blocking(WS2812Driver *pDriver, const uint32_t *pArgbBuffer);
void ws2812_driver_deinit(WS2812Driver **ppDriver);
