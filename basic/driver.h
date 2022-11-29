#pragma once

#include <stdlib.h>
#include "pico/stdlib.h"
#include <stdint.h>

typedef struct
{
    size_t ledCount;
    uint dmaChannel;
    uint16_t *dataBuffer;
} WS2812Driver;

void ws2812_driver_init(WS2812Driver *driver, size_t ledCount);
void ws2812_driver_submit_argb_buffer_blocking(WS2812Driver *driver, const uint32_t *argbBuffer);
void ws2812_driver_deinit(WS2812Driver *driver);
