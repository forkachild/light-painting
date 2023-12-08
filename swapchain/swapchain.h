#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <stdlib.h>

#define SWAPCHAIN_UNINIT ((swapchain_context_t *)NULL)

typedef struct swapchain_context swapchain_context_t;

int swapchain_init(swapchain_context_t **context, size_t buffer_size);

/**
 * Producer side
 */
void *swapchain_producer_buffer(swapchain_context_t *context);
void swapchain_producer_swap(swapchain_context_t *context);

/**
 * Consumer side
 */
void *swapchain_consumer_buffer(swapchain_context_t *context);
void swapchain_consumer_swap(swapchain_context_t *context);

void swapchain_deinit(swapchain_context_t **context);

#endif