#pragma once

#include "pico/types.h"
#include <stdlib.h>

typedef struct INMP441SwapchainNode INMP441SwapchainNode;
typedef struct INMP441Swapchain INMP441Swapchain;

struct INMP441Swapchain {
    INMP441SwapchainNode *cursor;
    uint buffer_size;
    uint count;
};

struct INMP441SwapchainNode {
    uint index;
    uint32_t *p_buffer;
    INMP441SwapchainNode *next;
    INMP441SwapchainNode *prev;
};

/**
 * @brief Initialize the chain before use. Must be called before calling any
 * other swapchain_*() methods
 *
 * @param chain
 * @param buffer_size
 * @param count
 */
static void swapchain_init(INMP441Swapchain *chain, uint buffer_size,
                           uint count) {
    uint i;

    chain->cursor =
        (INMP441SwapchainNode *)malloc(count * sizeof(INMP441SwapchainNode));
    chain->buffer_size = buffer_size;
    chain->count = count;

    for (i = 0; i < count; i++) {
        INMP441SwapchainNode *current = &chain->cursor[i];
        INMP441SwapchainNode *next = &chain->cursor[(i + 1) % count];
        current->index = i + 1;
        current->p_buffer = (uint32_t *)malloc(buffer_size * sizeof(uint32_t));
        current->next = next;
        next->prev = current;
    }
}

/**
 * @brief Get the size of each buffer contained inside each node
 *
 * @param chain
 * @return uint
 */
static inline uint swapchain_get_buffer_size(INMP441Swapchain *chain) {
    return chain->buffer_size;
}

/**
 * @brief Get the number of nodes in the swapchain
 *
 * @param chain
 * @return uint
 */
static inline uint swapchain_get_count(INMP441Swapchain *chain) {
    return chain->count;
}

/**
 * @brief Borrow a Node for writing to it
 *
 *  Borrows the node next to the cursor.
 *  Keeps cursor unchanged.
 *
 * @param chain
 * @return Node*
 */
static inline INMP441SwapchainNode *
swapchain_borrow_for_write(INMP441Swapchain *chain) {
    INMP441SwapchainNode *taken;

    if ((taken = chain->cursor->next) == chain->cursor)
        return NULL;

    taken->prev->next = taken->next;
    taken->next->prev = taken->prev;

    return taken;
}

/**
 * @brief Return a Node after writing
 *
 * Inserts the node next to the cursor.
 * Moves the cursor to the inserted node.
 *
 * @param chain
 * @param node
 */
static inline void swapchain_return_after_write(INMP441Swapchain *chain,
                                                INMP441SwapchainNode *node) {
    INMP441SwapchainNode *prev = chain->cursor;
    INMP441SwapchainNode *next = chain->cursor->next;

    // Insert the node
    prev->next = node;
    node->prev = prev;
    node->next = next;
    next->prev = node;

    // Change the cursor
    chain->cursor = node;
}

/**
 * @brief Borrow a Node for reading
 *
 *  Borrows the node at the cursor.
 *  Moves cursor to the next node.
 *
 * @param chain
 * @return Node*
 */
static inline INMP441SwapchainNode *
swapchain_borrow_for_read(INMP441Swapchain *chain) {
    INMP441SwapchainNode *taken;

    if ((taken = chain->cursor) == chain->cursor->next)
        return NULL;

    taken->prev->next = taken->next;
    taken->next->prev = taken->prev;
    chain->cursor = taken->next;

    return taken;
}

/**
 * @brief Return a Node after reading
 *
 * Inserts the node next to the cursor.
 * Keeps cursor unchanged.
 *
 * @param chain
 * @param node
 */
static inline void swapchain_return_after_read(INMP441Swapchain *chain,
                                               INMP441SwapchainNode *node) {
    INMP441SwapchainNode *prev = chain->cursor;
    INMP441SwapchainNode *next = chain->cursor->next;

    prev->next = node;
    node->prev = prev;
    node->next = next;
    next->prev = node;
}

/**
 * @brief Destroy the chain. Renders the chain useless after this call. Any
 * further calls to swapchain_*() methods after this call is undefined
 * behaviour.
 *
 * @param chain Pointer to the chain
 */
static void swapchain_deinit(INMP441Swapchain *chain) {
    uint i;

    if (!chain)
        return;

    for (i = 0; i < chain->count; i++) {
        free(chain->cursor[i].p_buffer);
    }

    free(chain->cursor);
}

/**
 * @brief Returns the raw pointer in memory for the audio buffer
 *
 * @param p_frame The pointer to the initialized audio frame in memory
 * @return uint32_t* The pointer to the start of the buffer
 */
static inline uint32_t *swapchain_node_get_ptr(INMP441SwapchainNode *p_node) {
    return p_node->p_buffer;
}
