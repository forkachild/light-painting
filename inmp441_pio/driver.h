#pragma once

#include "pico/types.h"

/**
 * @brief Opaque handle for the driver
 */
typedef struct INMP441PioDriver INMP441PioDriver;
typedef struct INMP441PioDriverBuffer INMP441PioDriverBuffer;

void inmp441_pio_driver_init(INMP441PioDriver **ppDriver, uint ctrl_pin_start,
                             uint data_pin);
void inmp441_pio_driver_receive_blocking(INMP441PioDriver *pDriver,
                                         INMP441PioDriverBuffer *pBuffer,
                                         uint count);
void inmp441_pio_driver_deinit(INMP441PioDriver **ppDriver);
void inmp441_pio_driver_buffer_init(INMP441PioDriverBuffer **ppBuffer,
                                    uint count);
uint inmp441_pio_driver_buffer_get_data_count(INMP441PioDriverBuffer *pDriver);
uint32_t *
inmp441_pio_driver_buffer_get_data_ptr(INMP441PioDriverBuffer *pDriver);
void inmp441_pio_driver_buffer_deinit(INMP441PioDriverBuffer **ppBuffer);