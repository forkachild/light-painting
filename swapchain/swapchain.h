#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <stdlib.h>

#define DEFAULT_BUFFER_COUNT 3
#define DEFAULT_RING_SIZE 2

typedef struct {
    void *mem;
    void *buffer_chain[DEFAULT_BUFFER_COUNT];
} swapchain_t;

int swapchain_init(swapchain_t *this, size_t buffer_size);

/**
 * Producer side
 */
void *swapchain_producer_buffer(swapchain_t *this);
void swapchain_producer_swap(swapchain_t *this);

/**
 * Consumer side
 */
void *swapchain_consumer_buffer(swapchain_t *this);
void swapchain_consumer_swap(swapchain_t *this);

void swapchain_deinit(swapchain_t *this);

#endif