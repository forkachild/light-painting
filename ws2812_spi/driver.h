#pragma once

#include <stdlib.h>
#include "pico/stdlib.h"
#include <stdint.h>

typedef struct WS2812SpiDriver WS2812SpiDriver;

void ws2812_spi_driver_init(WS2812SpiDriver **ppDriver, size_t ledCount);
void ws2812_spi_driver_submit_argb_buffer_blocking(WS2812SpiDriver *pDriver, const uint32_t *pArgbBuffer);
void ws2812_spi_driver_deinit(WS2812SpiDriver **ppDriver);
