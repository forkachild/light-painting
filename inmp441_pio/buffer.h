#pragma once

#include "pico/types.h"

typedef struct INMP441PioBuffer INMP441PioBuffer;

void inmp441_pio_buffer_init(INMP441PioBuffer **pp_buffer, uint count);
uint inmp441_pio_buffer_get_data_count(INMP441PioBuffer *p_driver);
uint32_t *inmp441_pio_buffer_get_data_ptr(INMP441PioBuffer *p_buffer);
uint inmp441_pio_buffer_get_trans_count(INMP441PioBuffer *p_buffer);
uint32_t *inmp441_pio_driver_buffer_get_trans_ptr(INMP441PioBuffer *p_buffer);
void inmp441_pio_buffer_deinit(INMP441PioBuffer **pp_buffer);