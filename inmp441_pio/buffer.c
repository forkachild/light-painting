#include <stdlib.h>

#include "buffer.h"

#define RX_FIFO_STALE_WORDS 4
#define INITIAL_BAD_SAMPLE 1
#define TOTAL_WORDS_SKIP (RX_FIFO_STALE_WORDS + INITIAL_BAD_SAMPLE)

struct INMP441PioBuffer {
    uint count;
    uint32_t *p_padded_buffer;
};

void inmp441_pio_buffer_init(INMP441PioBuffer **pp_buffer, uint count) {
    INMP441PioBuffer *buffer = malloc(sizeof(INMP441PioBuffer));
    buffer->count = count + TOTAL_WORDS_SKIP;
    buffer->p_padded_buffer =
        malloc((count + TOTAL_WORDS_SKIP) * sizeof(uint32_t));
    *pp_buffer = buffer;
}

uint inmp441_pio_buffer_get_data_count(INMP441PioBuffer *p_buffer) {
    return p_buffer->count - TOTAL_WORDS_SKIP;
}

uint32_t *inmp441_pio_buffer_get_data_ptr(INMP441PioBuffer *p_buffer) {
    return &p_buffer->p_padded_buffer[TOTAL_WORDS_SKIP];
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

    free(buffer);
    *pp_buffer = NULL;
}