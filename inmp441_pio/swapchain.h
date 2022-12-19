#pragma once

#include <stdlib.h>

#include "pico/critical_section.h"
#include "swapchain.h"

typedef struct {
    uint32_t *p_buffer;
    uint buffer_size;
    bool is_obtained;
} INMP441AudioFrame;

typedef struct {
    INMP441AudioFrame *p_frames;
    uint frame_count;
    critical_section_t lock;
} INMP441Swapchain;

/**
 * @brief Initializes the swapchain with the number of buffers
 *
 * @param pp_swapchain Address to a pointer to a struct in memory
 * @param count The number of buffers in the swapchain
 */
static void inmp441_swapchain_init(INMP441Swapchain **pp_swapchain,
                                   uint buffer_size, uint frame_count) {
    INMP441Swapchain *swapchain = NULL;
    INMP441AudioFrame *frames = NULL;
    uint i;

    if (*pp_swapchain)
        return;

    if (buffer_size == 0)
        return;

    if (frame_count < 2)
        return;

    frames = malloc(frame_count * sizeof(INMP441AudioFrame));

    for (i = 0; i < frame_count; i++) {
        INMP441AudioFrame *frame = &frames[i];
        frame->p_buffer = malloc(buffer_size * sizeof(uint32_t));
        frame->buffer_size = buffer_size;
        frame->is_obtained = false;
    }

    swapchain = malloc(sizeof(INMP441Swapchain));

    swapchain->p_frames = frames;
    swapchain->frame_count = frame_count;
    critical_section_init(&swapchain->lock);

    *pp_swapchain = swapchain;
}

/**
 * @brief Returns an index in the swapchain locked for use
 *
 * @param p_swapchain The pointer to the initialized driver in memory
 * @return int An index in the swapchain. -1 if none are free
 */
static inline int
inmp441_swapchain_obtain_idx_blocking(INMP441Swapchain *p_swapchain) {
    int index = -1;

    critical_section_enter_blocking(&p_swapchain->lock);

    // Frame switching taking place. Must be atomic, otherwise
    // invalid data read/write may happen
    for (uint i = 0; i < p_swapchain->frame_count; i++) {
        INMP441AudioFrame *frame = &p_swapchain->p_frames[i];
        if (!frame->is_obtained) {
            frame->is_obtained = true;
            index = (int)i;
            break;
        }
    }

    critical_section_exit(&p_swapchain->lock);

    return index;
}

/**
 * @brief Submits the index and unlocks it in the swapchain
 *
 * @param p_swapchain The pointer to the initialized driver in memory
 * @param idx The index of the frame being submitted
 */
static inline void inmp441_swapchain_submit_idx(INMP441Swapchain *p_swapchain,
                                                uint idx) {
    critical_section_enter_blocking(&p_swapchain->lock);
    p_swapchain->p_frames[idx].is_obtained = false;
    critical_section_exit(&p_swapchain->lock);
}

/**
 * @brief Get the actual frame referred by the index. Does not check the index
 * for range validity, hence unsafe
 *
 * @param p_swapchain The pointer to the initialized driver in memory
 * @param idx The index of the frame to return
 * @return INMP441AudioFrame* The frame struct pointer
 */
static inline INMP441AudioFrame *
inmp441_swapchain_get_frame_unsafe(INMP441Swapchain *p_swapchain, uint idx) {
    return &p_swapchain->p_frames[idx];
}

/**
 * @brief Destroys the swapchain and deletes from memory
 *
 * @param pp_swapchain The address to a pointer of the driver
 */
static void inmp441_swapchain_deinit(INMP441Swapchain **pp_swapchain) {
    INMP441Swapchain *swapchain;
    uint i;

    if (!(swapchain = *pp_swapchain))
        return;

    for (i = 0; i < swapchain->frame_count; i++) {
        free(&swapchain->p_frames[i].p_buffer);
    }

    free(swapchain->p_frames);
    free(swapchain);

    *pp_swapchain = NULL;
}

/**
 * @brief Returns the size of the raw buffer underneath
 *
 * @param p_frame The pointer to the initialized audio frame in memory
 * @return uint The size of the raw buffer
 */
static inline uint inmp441_audio_frame_get_size(INMP441AudioFrame *p_frame) {
    return p_frame->buffer_size;
}

/**
 * @brief Returns the raw pointer in memory for the audio buffer
 *
 * @param p_frame The pointer to the initialized audio frame in memory
 * @return uint32_t* The pointer to the start of the buffer
 */
static inline uint32_t *
inmp441_audio_frame_get_ptr_unsafe(INMP441AudioFrame *p_frame) {
    return p_frame->p_buffer;
}
