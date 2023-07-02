#include "components/buffer.h"

#include <stdio.h>
#include <stdlib.h>

void *async_buffer_node_data_ptr(AsyncBufferNode *node) { return node->data; }

void async_buffer_init(AsyncBuffer *buffer, size_t size) {
    for (int i = 0; i < ASYNC_BUFFER_LEN - 1; i++) {
        buffer->nodes[i].next = &buffer->nodes[i + 1];
        buffer->nodes[i + 1].prev = &buffer->nodes[i];
    }

    void *buffer_mem = malloc(ASYNC_BUFFER_LEN * size);

    for (int i = 0; i < ASYNC_BUFFER_LEN; i++) {
        buffer->nodes[i].data = buffer_mem + size * i;
    }

    buffer->mem = buffer_mem;
    buffer->top = &buffer->nodes[0];
    buffer->bottom = &buffer->nodes[ASYNC_BUFFER_LEN - 1];
}

// Takes bottom node
// Moves bottom up
AsyncBufferNode *async_buffer_producer_obtain(AsyncBuffer *buffer) {
    AsyncBufferNode *bottom = buffer->bottom;

    bottom->prev->next = NULL;
    buffer->bottom = bottom->prev;
    return bottom;
}

// Inserts before top
// Moves top up
void async_buffer_producer_submit(AsyncBuffer *buffer, AsyncBufferNode *node) {
    node->next = buffer->top;
    buffer->top->prev = node;
    buffer->top = node;
}

// Takes top
// Moves top down
AsyncBufferNode *async_buffer_consumer_obtain(AsyncBuffer *buffer) {
    AsyncBufferNode *top = buffer->top;

    top->next->prev = NULL;
    buffer->top = top->next;

    return top;
}

// Insert after bottom
// Moves bottom down
void async_buffer_consumer_submit(AsyncBuffer *buffer, AsyncBufferNode *node) {
    node->prev = buffer->bottom;
    buffer->bottom->next = node;
    buffer->bottom = node;
}

void async_buffer_deinit(AsyncBuffer *buffer) {
    free(buffer->mem);

    for (int i = 0; i < ASYNC_BUFFER_LEN; i++) {
        AsyncBufferNode *node = &buffer->nodes[i];

        node->data = NULL;
        node->next = NULL;
        node->prev = NULL;
    }

    buffer->top = NULL;
    buffer->bottom = NULL;
}