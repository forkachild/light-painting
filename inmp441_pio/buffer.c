#include <stdlib.h>

#include "buffer.h"

struct INMP441AudioBuffer {
    uint size;
    uint32_t *p_mem;
};

void inmp441_audio_buffer_init(INMP441AudioBuffer **pp_buffer, uint size) {
    INMP441AudioBuffer *buffer;

    if (*pp_buffer)
        return;

    buffer = malloc(sizeof(INMP441AudioBuffer));

    buffer->size = size;
    buffer->p_mem = malloc(size * sizeof(uint32_t));

    *pp_buffer = buffer;
}

uint inmp441_audio_buffer_get_size(INMP441AudioBuffer *p_buffer) {
    return p_buffer->size;
}

uint32_t *inmp441_audio_buffer_get_ptr_unsafe(INMP441AudioBuffer *p_buffer) {
    return p_buffer->p_mem;
}

void inmp441_audio_buffer_deinit(INMP441AudioBuffer **pp_buffer) {
    INMP441AudioBuffer *buffer;

    if (!(buffer = *pp_buffer))
        return;

    free(buffer->p_mem);
    free(buffer);

    *pp_buffer = NULL;
}