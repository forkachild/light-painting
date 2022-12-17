#include <stdlib.h>

#include "buffer.h"

// To flush out the stale 8 words buffered in the RX FIFO
// of the PIO
#define RX_FIFO_STALE_WORDS 8

struct INMP441PioBuffer {
    uint count;
    uint32_t *p_padded_buffer;
};

void inmp441_pio_buffer_init(INMP441PioBuffer **pp_buffer, uint count) {
    INMP441PioBuffer *buffer;

    if (*pp_buffer)
        return;

    buffer = malloc(sizeof(INMP441PioBuffer));

    buffer->count = count + RX_FIFO_STALE_WORDS;
    buffer->p_padded_buffer =
        malloc((count + RX_FIFO_STALE_WORDS) * sizeof(uint32_t));

    *pp_buffer = buffer;
}

uint inmp441_pio_buffer_get_data_count(INMP441PioBuffer *p_buffer) {
    return p_buffer->count - RX_FIFO_STALE_WORDS;
}

uint32_t *inmp441_pio_buffer_get_data_ptr(INMP441PioBuffer *p_buffer) {
    return &p_buffer->p_padded_buffer[RX_FIFO_STALE_WORDS];
}

uint inmp441_pio_buffer_get_trans_count(INMP441PioBuffer *p_buffer) {
    return p_buffer->count;
}

uint32_t *inmp441_pio_buffer_get_trans_ptr(INMP441PioBuffer *p_buffer) {
    return p_buffer->p_padded_buffer;
}

void inmp441_pio_buffer_deinit(INMP441PioBuffer **pp_buffer) {
    INMP441PioBuffer *buffer;

    if (!(buffer = *pp_buffer))
        return;

    free(buffer->p_padded_buffer);
    free(buffer);

    *pp_buffer = NULL;
}