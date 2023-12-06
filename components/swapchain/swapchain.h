#pragma once

#include "pico/types.h"
#include <stdbool.h>

typedef struct SwapchainNode SwapchainNode;
typedef struct Swapchain Swapchain;

struct Swapchain {
    SwapchainNode *cursor;
    uint size;
    uint length;
};

struct SwapchainNode {
    void *buffer;
    bool borrowed;
    SwapchainNode *next;
    SwapchainNode *prev;
};

/**
 * @brief Initialize the chain before use. Must be called before calling any
 * other swapchain_*() methods
 *
 * @param chain Pointer to an initialized swapchain in memory
 * @param size Size of buffer in each node (in bytes)
 * @param length Number of swappable nodes
 */
void swapchain_init(Swapchain *chain, uint size, uint length);

/**
 * @brief Get the size of each buffer contained inside each node
 *
 * @param chain
 * @return uint
 */
uint swapchain_get_size(const Swapchain *chain);

/**
 * @brief Get the number of nodes in the swapchain
 *
 * @param chain
 * @return uint
 */
uint swapchain_get_length(const Swapchain *chain);

/**
 * @brief Tries to borrow a Node for writing to it. Returns NULL if none
 * available
 *
 *  Borrows the node next to the cursor.
 *  Keeps cursor unchanged.
 *
 * @param chain
 * @return Node*
 */
SwapchainNode *swapchain_try_borrow_for_write(Swapchain *chain);

/**
 * @brief Return a Node after reading
 *
 * Inserts the node next to the cursor.
 * Keeps cursor unchanged.
 *
 * @param chain
 * @param node
 */
void swapchain_return_after_write(Swapchain *chain, SwapchainNode *node);

/**
 * @brief Borrow a Node for reading
 *
 * Borrows the node at the cursor.
 * Moves cursor to the next node.
 *
 * @param chain
 * @return Node*
 */
SwapchainNode *swapchain_try_borrow_for_read(Swapchain *chain);

/**
 * @brief Return a Node after reading
 *
 * Inserts the node next to the cursor.
 * Keeps cursor unchanged.
 *
 * @param chain
 * @param node
 */
void swapchain_return_after_read(Swapchain *chain, SwapchainNode *node);

/**
 * @brief Destroy the chain
 *
 * Renders the chain useless after this call. Any
 * further calls to swapchain_*() methods after this call is undefined
 * behaviour.
 *
 * @param chain Pointer to the chain
 */
void swapchain_deinit(Swapchain *chain);

/**
 * @brief Returns the raw pointer in memory for the audio buffer
 *
 * @param node The pointer to the initialized audio frame in memory
 * @return The pointer to the start of the buffer
 */
void *swapchain_node_get_p_buffer(const SwapchainNode *node);
