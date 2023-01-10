#pragma once

#include "pico/types.h"
#include "types.h"

typedef struct SwapchainNode SwapchainNode;
typedef struct Swapchain Swapchain;

struct Swapchain {
    SwapchainNode *cursor;
    uint buffer_size;
    uint nodes;
};

struct SwapchainNode {
    uint index;
    void *buffer;
    SwapchainNode *next;
    SwapchainNode *prev;
};

/**
 * @brief Initialize the chain before use. Must be called before calling any
 * other swapchain_*() methods
 *
 * @param chain Pointer to an initialized swapchain in memory
 * @param buffer_size Size of buffer in each node (in bytes)
 * @param nodes Number of swappable nodes
 */
void swapchain_init(Swapchain *chain, uint buffer_size, uint nodes);

/**
 * @brief Get the size of each buffer contained inside each node
 *
 * @param chain
 * @return uint
 */
uint swapchain_get_buffer_size(Swapchain *chain);

/**
 * @brief Get the number of nodes in the swapchain
 *
 * @param chain
 * @return uint
 */
uint swapchain_get_nodes(Swapchain *chain);

/**
 * @brief Borrow a Node for writing to it
 *
 *  Borrows the node next to the cursor.
 *  Keeps cursor unchanged.
 *
 * @param chain
 * @return Node*
 */
SwapchainNode *swapchain_borrow_for_write(Swapchain *chain);

/**
 * @brief Return a Node after writing
 *
 * Inserts the node next to the cursor.
 * Moves the cursor to the inserted node.
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
SwapchainNode *swapchain_borrow_for_read(Swapchain *chain);

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
 * @param frame The pointer to the initialized audio frame in memory
 * @return uint32_t* The pointer to the start of the buffer
 */
void *swapchain_node_get_buffer_ptr(SwapchainNode *node);
