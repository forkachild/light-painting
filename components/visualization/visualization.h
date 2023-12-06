#pragma once

#include "buffer.h"
#include <pico/types.h>

typedef struct Visualization {
    size_t sample_count;
    size_t led_count;
    uint led_pin;
    uint audio_sck_pin;
    uint audio_ws_pin;
    uint audio_lr_pin;
    AsyncBuffer in_buffer;
    AsyncBuffer out_buffer;

} Visualization;

void visualization_init(Visualization *visualization, size_t sample_count,
                        size_t led_count, uint led_pin, uint audio_sck_pin,
                        uint audio_ws_pin, uint audio_lr_pin);
void visualization_start(Visualization *visualization);
void visualization_stop(Visualization *visualization);
void visualization_deinit(Visualization *visualization);