#include "swapchain.h"
#include <pico/critical_section.h>

#define DEFAULT_BUFFER_COUNT 3
#define DEFAULT_RING_SIZE 2

#define LEFT_INDEX 0
#define RIGHT_INDEX 1
#define MID_INDEX 2

struct swapchain_context {
    void *mem;
    void *buffer_chain[DEFAULT_BUFFER_COUNT];
};

int swapchain_init(swapchain_context_t **context, size_t buffer_size) {
    if (*context != SWAPCHAIN_UNINIT)
        return -1;

    swapchain_context_t *c = malloc(sizeof(swapchain_context_t));

    if (c == NULL)
        return -1;

    void *alloc = malloc(DEFAULT_BUFFER_COUNT * buffer_size);
    if (alloc == NULL) {
        free(c);
        return -1;
    }

    for (size_t i = 0; i < DEFAULT_BUFFER_COUNT; i++)
        c->buffer_chain[i] = (void *)((size_t)alloc + (i * buffer_size));

    c->mem = alloc;

    *context = c;

    return 1;
}

void *swapchain_producer_buffer(swapchain_context_t *context) {
    return context->buffer_chain[LEFT_INDEX];
}

void swapchain_producer_swap(swapchain_context_t *context) {
#ifdef SWAPCHAIN_CRITICAL_SECTION
    uint32_t saved_irq = save_and_disable_interrupts();
#endif

    swapchain_context_t *temp = context->buffer_chain[MID_INDEX];
    context->buffer_chain[MID_INDEX] = context->buffer_chain[LEFT_INDEX];
    context->buffer_chain[LEFT_INDEX] = temp;

#ifdef SWAPCHAIN_CRITICAL_SECTION
    restore_interrupts(saved_irq);
#endif
}

void *swapchain_consumer_buffer(swapchain_context_t *context) {
    return context->buffer_chain[RIGHT_INDEX];
}

void swapchain_consumer_swap(swapchain_context_t *context) {
#ifdef SWAPCHAIN_CRITICAL_SECTION
    uint32_t saved_irq = save_and_disable_interrupts();
#endif

    swapchain_context_t *temp = context->buffer_chain[MID_INDEX];
    context->buffer_chain[MID_INDEX] = context->buffer_chain[RIGHT_INDEX];
    context->buffer_chain[RIGHT_INDEX] = temp;

#ifdef SWAPCHAIN_CRITICAL_SECTION
    restore_interrupts(saved_irq);
#endif
}

void swapchain_deinit(swapchain_context_t **context) {
    swapchain_context_t *c = *context;

    if (c == SWAPCHAIN_UNINIT)
        return;

    free(c->mem);
    free(c);

    *context = SWAPCHAIN_UNINIT;
}
