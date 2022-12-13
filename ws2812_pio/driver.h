#pragma once

#include "pico/types.h"

typedef struct WS2812PioDriver WS2812PioDriver;

void ws2812_pio_driver_init(WS2812PioDriver **pp_driver, uint pin, uint count);
void ws2812_pio_driver_submit_buffer_blocking(WS2812PioDriver *p_driver,
                                              const uint32_t *p_buffer);
void ws2812_pio_driver_deinit(WS2812PioDriver **pp_driver);