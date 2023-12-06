#include "swapchain.h"
#include "pico/types.h"
#include <stdlib.h>

/**
 * @brief Initialize the chain before use
 *
 * Must be called before calling any other swapchain_*()` methods.
 *
 * @param chain Pointer to an initialized swapchain in memory
 * @param buffer_size Size of buffer in each node
 * @param nodes Number of swappable nodes
 */
void swapchain_init(Swapchain *chain, uint size, uint length) {
    uint i;

    void *p_mem = malloc(length * (sizeof(SwapchainNode) + size));

    chain->cursor = p_mem;
    chain->size = size;
    chain->length = length;

    void *p_buffer_mem = p_mem + (length * sizeof(SwapchainNode));

    for (i = 0; i < length; i++) {
        SwapchainNode *current = &chain->cursor[i];
        SwapchainNode *next = &chain->cursor[(i + 1) % length];

        current->borrowed = false;
        current->buffer = p_buffer_mem;
        current->next = next;
        next->prev = current;

        p_buffer_mem += size;
    }
}

/**
 * @brief Get the size of each buffer contained inside each node
 *
 * @param chain
 * @return uint
 */
uint swapchain_get_size(const Swapchain *chain) { return chain->size; }

/**
 * @brief Get the number of nodes in the swapchain
 *
 * @param chain
 * @return uint
 */
uint swapchain_get_length(const Swapchain *chain) { return chain->length; }

/**
 * @brief Borrow a Node for writing to it
 *
 *   O
 *  / \
 * O - O
 *
 *   O
 *  / \
 * O - X
 *
 *  Tries to borrow the node next to the cursor.
 *  Keeps cursor unchanged.
 *
 * @param chain
 * @return The borrowed node
 */
SwapchainNode *swapchain_try_borrow_for_write(Swapchain *chain) {
    SwapchainNode *node = chain->cursor->next;

    if (node->borrowed)
        return NULL;

    node->borrowed = true;

    return node;
}

/**
 * @brief Return a Node after writing
 *
 * Inserts the node next to the cursor.
 * Keeps cursor unchanged.
 *
 * @param chain
 * @param node
 */
void swapchain_return_after_write(Swapchain *chain, SwapchainNode *node) {
    node->borrowed = false;
    chain->cursor = chain->cursor->next;
}

/**
 * @brief Borrow a Node for reading
 *
 *   O
 *  / \
 * O - O
 *
 *   X
 *  / \
 * O - O
 *
 *   O
 *  / \
 * X - O
 *
 * Borrows the node at the cursor.
 * Moves cursor to the next node.
 *
 * @param chain
 * @return Node*
 */
SwapchainNode *swapchain_try_borrow_for_read(Swapchain *chain) {
    SwapchainNode *node = chain->cursor;

    if (node->borrowed)
        return NULL;

    node->borrowed = true;
    chain->cursor = node->next;

    return node;
}

/**
 * @brief Return a Node after reading
 *
 *   O
 *  / \
 * O - O
 *
 * Inserts the node next to the cursor.
 * Keeps cursor unchanged.
 *
 * @param chain
 * @param node
 */
void swapchain_return_after_read(Swapchain __unused *chain,
                                 SwapchainNode *node) {
    node->borrowed = false;
}

/**
 * @brief Destroy the chain
 *
 * Renders the chain useless after this call. Any
 * further calls to swapchain_*() methods after this call is undefined
 * behaviour.
 *
 * @param chain Pointer to the chain
 */
void swapchain_deinit(Swapchain *chain) {
    if (!chain)
        return;

    // Unified memory model
    free(chain->cursor);

    chain->cursor = NULL;
    chain->length = 0;
    chain->size = 0;
}

/**
 * @brief Returns the raw pointer in memory for the audio buffer
 *
 * @param frame The pointer to the initialized audio frame in memory
 * @return void* The pointer to the start of the buffer
 */
void *swapchain_node_get_p_buffer(const SwapchainNode *node) {
    return node->buffer;
}
