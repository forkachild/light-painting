#include "components/visualization.h"

#include "components/buffer.h"
#include <stdlib.h>

void visualization_init(Visualization *v, size_t sample_count, size_t led_count,
                        uint led_pin, uint audio_sck_pin, uint audio_ws_pin,
                        uint audio_lr_pin) {
    v->sample_count = sample_count;
    v->led_count = led_count;
    v->led_pin = led_pin;
    v->audio_sck_pin = audio_sck_pin;
    v->audio_ws_pin = audio_ws_pin;
    v->audio_lr_pin = audio_lr_pin;
}

void visualization_start(Visualization *v) {
    AsyncBuffer in_buffer, out_buffer;

    async_buffer_init(&in_buffer, v->sample_count * sizeof(float));
    async_buffer_init(&out_buffer, v->led_count * sizeof(uint32_t));
}

void visualization_stop(Visualization __unused *visualization) {}

void visualization_deinit(Visualization __unused *visualization) {}

void pseudo() {
    // Prepare async buffers
    // Configure and start input
    // Configure and start output

    // Start loop
    // Fetch frame from input
    // Scale into float buffer (needs another buffer)
    // Apply smoothing convolution (needs another buffer)
    // Apply amplification
    // Apply envelope (needs another buffer)
    // Apply FFT
    // Linearly interpolate FFT to output color (needs another buffer)
    // Send frame to output
    // End loop

    // Translating the above lines into code
}