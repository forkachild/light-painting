#include "swapchain.h"
#include <pico/critical_section.h>

#define PRODUCER_INDEX 0
#define CONSUMER_INDEX 1
#define SHARED_INDEX 2

int swapchain_init(swapchain_t *this, size_t buffer_size) {
    void *alloc = malloc(DEFAULT_BUFFER_COUNT * buffer_size);
    if (alloc == NULL)
        return -1;

    for (size_t i = 0; i < DEFAULT_BUFFER_COUNT; i++)
        this->buffer_chain[i] = (void *)((size_t)alloc + (i * buffer_size));

    this->mem = alloc;

    return 1;
}

void *swapchain_producer_buffer(swapchain_t *this) {
    return this->buffer_chain[PRODUCER_INDEX];
}

void swapchain_producer_swap(swapchain_t *this) {
#ifdef SWAPCHAIN_CRITICAL_SECTION
    uint32_t saved_irq = save_and_disable_interrupts();
#endif

    swapchain_t *temp = this->buffer_chain[SHARED_INDEX];
    this->buffer_chain[SHARED_INDEX] = this->buffer_chain[PRODUCER_INDEX];
    this->buffer_chain[PRODUCER_INDEX] = temp;

#ifdef SWAPCHAIN_CRITICAL_SECTION
    restore_interrupts(saved_irq);
#endif
}

void *swapchain_consumer_buffer(swapchain_t *this) {
    return this->buffer_chain[CONSUMER_INDEX];
}

void swapchain_consumer_swap(swapchain_t *this) {
#ifdef SWAPCHAIN_CRITICAL_SECTION
    uint32_t saved_irq = save_and_disable_interrupts();
#endif

    swapchain_t *temp = this->buffer_chain[SHARED_INDEX];
    this->buffer_chain[SHARED_INDEX] = this->buffer_chain[CONSUMER_INDEX];
    this->buffer_chain[CONSUMER_INDEX] = temp;

#ifdef SWAPCHAIN_CRITICAL_SECTION
    restore_interrupts(saved_irq);
#endif
}

void swapchain_deinit(swapchain_t *this) { free(this->mem); }
