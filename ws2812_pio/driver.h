#pragma once

#include "pico/types.h"

typedef struct WS2812PioDriver WS2812PioDriver;

void ws2812_pio_driver_init(WS2812PioDriver **ppDriver, uint pin, uint count);
void ws2812_pio_driver_submit_buffer_blocking(WS2812PioDriver *pDriver,
                                              const uint32_t *pBuffer);
void ws2812_pio_driver_deinit(WS2812PioDriver **ppDriver);