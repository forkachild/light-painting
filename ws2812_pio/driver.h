#pragma once

#include "pico/types.h"

/**
 * @brief The WS2812 PIO Driver opaque struct
 */
typedef struct WS2812PioDriver WS2812PioDriver;

/**
 * @brief Allocate the driver and initialize it. Must be called before any other
 * method can be called
 *
 * @param pp_driver The pointer to assign the address of the allocated driver
 * @param count The number of WS2812 serial LEDs to control
 * @param pin Which GPIO to use to transmit the WS2812 protocol
 * @param dma Whether DMA should be used
 */
void ws2812_pio_driver_init(WS2812PioDriver **pp_driver, uint count, uint pin,
                            bool dma);

uint ws2812_pio_driver_get_count(WS2812PioDriver *p_driver);

/**
 * @brief Blocking submit a 1D pixel buffer of same size as LED count
 *
 * @param p_driver The driver address in memory
 * @param p_buffer The pixel buffer address in memory
 */
void ws2812_pio_driver_submit_buffer_blocking(WS2812PioDriver *p_driver,
                                              const uint32_t *p_buffer);

/**
 * @brief Destroy the driver and deallocate the memory. This must be called when
 * the driver will not be used anymore.
 *
 * @param pp_driver The pointer to the address of the driver in memory
 */
void ws2812_pio_driver_deinit(WS2812PioDriver **pp_driver);