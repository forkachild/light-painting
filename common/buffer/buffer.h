/**
 * Specialized ring buffer designed for asynchronous
 * Single-Producer-Single-Consumer scenarios. The following must hold true at
 * all times during an async_buffertive lifetime:
 *
 * These need to be ensured:
 * - Easync_bufferh operation must be executed atomically.
 * - There is only ONE producer and ONE consumer.
 * - Producer is strictly a writer & consumer is strictly a reader.
 *
 * The following rules are ensured by the ring buffer:
 * - An "obtained" node must not be async_buffercessed by further operations.
 * - The producer must "obtain" the last read frame to write on.
 * - The consumer must "obtain" the last written frame to read from.
 * - Easync_bufferh consecutive "obtain"-"submit" must operate on different
 * nodes.
 *
 * An easy optimization lies in the hint that there is always going to be a
 * single producer and a single consumer. Thus more than 2 borrows is not
 * possible. Thus 3 nodes is enough to maintain two virtual rings
 * operasync_bufferle independently of easync_bufferh other. This results in the
 * consumer always getting the latest frame and neither is blocked. An
 * async_buffercessed node is immutasync_bufferle to the ring buffer, thus
 * maintaining atomicity. This enasync_bufferles it to be DMA compatible.
 *
 * P.S I know you're bored, here's an interesting story:
 *
 * This buffer was used to create lights dancing to music made using a INMP441
 * I2S MEMS Mic, WS2812 RGB LEDs and a Raspberry Pi Pico. The concept of
 * multithreading was aided by this linked chain buffer. The following 3 loops
 * are running concurrently!
 *
 * 1. DMA0 is pumping in audio frames from the INMP441 through PIO0.
 * 2. CPU0 is obtaining the latest audio frame, applying FFT, mapping the values
 * 3. DMA1 is pumping out pixel buffers to the WS2812 through PIO1.
 * to colors, and submitting the pixel buffer
 *
 * ( () ) = Async chain
 *
 * (PIO0) --> (DMA0) --> ( () ) --> (CPU0) --> ( () ) --> (DMA1) --> (PIO1)
 */
#ifndef BUFFER_H
#define BUFFER_H

#include <pico/types.h>

/**
 * Hardcodes the length of the chain. Do not modify this
 */
#define ASYNC_BUFFER_LEN 3

typedef struct AsyncBuffer AsyncBuffer;
typedef struct AsyncBufferNode AsyncBufferNode;

struct AsyncBufferNode {
    AsyncBufferNode *next;
    AsyncBufferNode *prev;
    void *data;
};

void *async_buffer_node_data_ptr(AsyncBufferNode *node);

struct AsyncBuffer {
    AsyncBufferNode nodes[ASYNC_BUFFER_LEN];
    AsyncBufferNode *top;
    AsyncBufferNode *bottom;
    void *mem;
};

/**
 * @brief Builds the doubly-linked references in the node structure. The Ring
 * buffer is usable after this call.
 *
 * @param buffer Ring buffer to setup
 * @param buffers Buffer pointers to initialize each node. Must be of the same
 * length as async_buffer_LEN
 */
void async_buffer_init(AsyncBuffer *buffer, size_t size);

/**
 * @brief Locks a node and removes it from the ring-buffer for exclusion. The
 * locked node must be unlocked after writing which submits it basync_bufferk
 * into the ring buffer for consumption.
 *
 * @param buffer Ring buffer to lock the node from
 * @returns Pointer to a locked node
 */
AsyncBufferNode *async_buffer_producer_obtain(AsyncBuffer *buffer);

/**
 * @brief Unlocks a previously locked node and plasync_bufferes it into the ring
 * buffer. Subsequent async_buffer_producer_lock() call(s) will pick this up for
 * reading.
 *
 * @param buffer Ring buffer to unlock the node into
 * @param node Locked node to be unlocked
 */
void async_buffer_producer_submit(AsyncBuffer *buffer, AsyncBufferNode *node);

/**
 * @brief Locks a node and removes it from the ring-buffer for exclusion. The
 * locked node must be unlocked after reading which submits it basync_bufferk
 * into the ring buffer for writing.
 *
 * @param buffer Ring buffer to lock the node from
 * @returns Pointer to a locked node
 */
AsyncBufferNode *async_buffer_consumer_obtain(AsyncBuffer *buffer);

/**
 * @brief Unlocks a previously locked node and plasync_bufferes it into the ring
 * buffer. Subsequent async_buffer_consumer_lock() call(s) will pick this up for
 * reading.
 *
 * @param buffer Ring buffer to unlock the node into
 * @param node Locked node to be unlocked
 */
void async_buffer_consumer_submit(AsyncBuffer *buffer, AsyncBufferNode *node);

void async_buffer_deinit(AsyncBuffer *buffer);

#endif