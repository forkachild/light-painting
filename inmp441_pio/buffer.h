#pragma once

#include "buffer.h"
#include "pico/types.h"

typedef struct INMP441AudioBuffer INMP441AudioBuffer;

void inmp441_audio_buffer_init(INMP441AudioBuffer **pp_buffer, uint size);
uint inmp441_audio_buffer_get_size(INMP441AudioBuffer *p_driver);
uint32_t *inmp441_audio_buffer_get_ptr_unsafe(INMP441AudioBuffer *p_buffer);
void inmp441_audio_buffer_deinit(INMP441AudioBuffer **pp_buffer);