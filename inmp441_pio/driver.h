#pragma once

#include "buffer.h"
#include "pico/types.h"

/**
 * @brief Opaque handle for the driver
 */
typedef struct INMP441PioDriver INMP441PioDriver;

void inmp441_pio_driver_init(INMP441PioDriver **pp_driver, uint sck_pin,
                             uint ws_pin, uint data_pin, uint lr_config_pin);
void inmp441_pio_driver_receive_blocking(INMP441PioDriver *p_driver,
                                         INMP441PioBuffer *p_buffer);
void inmp441_pio_driver_deinit(INMP441PioDriver **pp_driver);
