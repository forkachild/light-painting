#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <stdlib.h>

typedef struct swapchain_context swapchain_context_t;

#define SWAPCHAIN_UNINIT ((swapchain_context_t *)NULL)

int swapchain_init(swapchain_context_t **context, size_t buffer_size);
void *swapchain_get_left_buffer(swapchain_context_t *context);
void *swapchain_get_right_buffer(swapchain_context_t *context);
void swapchain_swap_left_side(swapchain_context_t *context);
void swapchain_swap_right_side(swapchain_context_t *context);
void swapchain_deinit(swapchain_context_t **context);

#endif