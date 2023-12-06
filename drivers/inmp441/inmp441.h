#ifndef INMP441_PIO_H
#define INMP441_PIO_H

#include "components/buffer.h"
#include "types.h"

Result inmp441_init(uint samples, uint sck_pin, uint ws_pin, uint data_pin);

void inmp441_start_sampling();

void inmp441_stop_sampling();

AsyncBuffer *inmp441_get_async_buffer();

Result inmp441_deinit();

#endif
