#include "swapchain.h"

#define DEFAULT_BUFFER_COUNT 3

#define LEFT_INDEX 0
#define RIGHT_INDEX 1
#define MID_INDEX 2

struct swapchain_context {
    void *mem;
    void *buffer_chain[DEFAULT_BUFFER_COUNT];
};

int swapchain_init(swapchain_context_t **context, size_t buffer_size) {
    swapchain_context_t *c = malloc(sizeof(swapchain_context_t));
    if (c == NULL)
        return -1;

    void *alloc = malloc(DEFAULT_BUFFER_COUNT * buffer_size);
    if (alloc == NULL) {
        free(c);
        return -1;
    }

    for (size_t i = 0; i < DEFAULT_BUFFER_COUNT; i++)
        c->buffer_chain[i] = alloc + i * buffer_size;

    c->mem = alloc;
    *context = c;

    return 1;
}

void *swapchain_get_left_buffer(swapchain_context_t *context) {
    return context->buffer_chain[LEFT_INDEX];
}

void *swapchain_get_right_buffer(swapchain_context_t *context) {
    return context->buffer_chain[RIGHT_INDEX];
}

void swapchain_swap_left_side(swapchain_context_t *context) {
    swapchain_context_t *temp = context->buffer_chain[MID_INDEX];
    context->buffer_chain[MID_INDEX] = context->buffer_chain[LEFT_INDEX];
    context->buffer_chain[LEFT_INDEX] = temp;
}

void swapchain_swap_right_side(swapchain_context_t *context) {
    swapchain_context_t *temp = context->buffer_chain[MID_INDEX];
    context->buffer_chain[MID_INDEX] = context->buffer_chain[RIGHT_INDEX];
    context->buffer_chain[RIGHT_INDEX] = temp;
}

void swapchain_deinit(swapchain_context_t **context) {
    if (*context == SWAPCHAIN_UNINIT)
        return;

    free((**context).mem);
    free(*context);

    *context = SWAPCHAIN_UNINIT;
}
