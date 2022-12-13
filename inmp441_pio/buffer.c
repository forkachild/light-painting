#include "buffer.h"

#ifdef RX_FIFO_JOIN_TX
#define RX_FIFO_SKIP_STALE 8
#else
#define RX_FIFO_SKIP_STALE 4
#endif

struct INMP441PioBuffer {
    uint count;
    uint32_t *p_padded_buffer;
};

void inpm441_pio_buffer_init(INMP441PioBuffer **pp_buffer, uint count) {
    INMP441PioBuffer *buffer = malloc(sizeof(INMP441PioBuffer));
    buffer->count = count + RX_FIFO_SKIP_STALE;
    buffer->p_padded_buffer =
        malloc((count + RX_FIFO_SKIP_STALE) * sizeof(uint32_t));
    *pp_buffer = buffer;
}

uint inmp441_pio_buffer_get_data_count(INMP441PioBuffer *p_buffer) {
    return p_buffer->count - RX_FIFO_SKIP_STALE;
}

uint32_t *inmp441_pio_driver_buffer_get_data_ptr(INMP441PioBuffer *p_buffer) {
    return &p_buffer->p_padded_buffer[RX_FIFO_SKIP_STALE];
}

uint inmp441_pio_buffer_get_trans_count(INMP441PioBuffer *p_buffer) {
    return p_buffer->count;
}

uint32_t *inmp441_pio_driver_buffer_get_trans_ptr(INMP441PioBuffer *p_buffer) {
    return p_buffer->p_padded_buffer;
}

void inmp441_pio_driver_buffer_deinit(INMP441PioBuffer **pp_buffer) {
    INMP441PioBuffer *buffer;

    if (!(buffer = *pp_buffer))
        return;

    free(buffer);
    *pp_buffer = NULL;
}