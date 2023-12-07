#include "audio.h"
#include "canvas.h"
#include "fft.h"
#include "inmp441.h"
#include "pico/stdlib.h"
#include "pico/types.h"
#include "swapchain.h"
#include "ws2812.h"
#include <stdio.h>
#include <stdlib.h>

#define AUDIO_SAMPLE_COUNT 256
#define LED_COUNT 300

#define MIC_SCK_PIN 10
#define MIC_WS_PIN 11
#define MIC_DATA_PIN 12

#define LED_DATA_PIN 13

static argb_color_t magnitude_to_color(float magnitude) {
    if (magnitude < 0.f) {
        magnitude = 0.f;
    }

    if (magnitude > 1.f) {
        magnitude = 1.f;
    }

    return argb_color_from_rgb(0xFF, 0xFF, 0xFF);
}

static void visualizer_map_frequency_bits_to_pixels(float *frequency_bins,
                                                    size_t frequency_bin_count,
                                                    uint32_t *pixel_buffer,
                                                    size_t pixel_count) {
    if (frequency_bin_count > pixel_count) {
        float pitch = (float)frequency_bin_count / pixel_count;

        for (size_t pixel = 0; pixel < pixel_count; pixel++) {
            float index = pixel * pitch;
            size_t index_i = (size_t)index;
            float index_f = index - index_i;

            pixel_buffer[pixel] =
                argb_color_add(
                    magnitude_to_color((1.f - index_f) *
                                       frequency_bins[index_i]),
                    magnitude_to_color(index_f * frequency_bins[index_i + 1]))
                    .value;
        }
    } else if (frequency_bin_count < pixel_count) {
        float pitch = (float)frequency_bin_count / pixel_count;

        for (size_t i = 0; i < pixel_count; i++) {
            size_t bin = i * pitch;
            pixel_buffer[i] = magnitude_to_color(frequency_bins[bin]).value;
        }
    } else {
        for (size_t i = 0; i < pixel_count; i++)
            pixel_buffer[i] = magnitude_to_color(frequency_bins[i]).value;
    }
}

int main() {
    audio_context_t *audio;
    stdio_usb_init();

    if (!inmp441_init(AUDIO_SAMPLE_COUNT, MIC_SCK_PIN, MIC_WS_PIN,
                      MIC_DATA_PIN)) {
        printf("Could not initialize INMP441 driver");
        return EXIT_FAILURE;
    }

    if (!ws2812_init(LED_COUNT, LED_DATA_PIN)) {
        printf("Could not initialize WS2812 driver");
        return EXIT_FAILURE;
    }

    if (!audio_init(&audio, AUDIO_SAMPLE_COUNT)) {
        printf("Could not initialize audio");
        return EXIT_FAILURE;
    }

    inmp441_start_sampling();
    ws2812_start_transmission();

    while (1) {
        inmp441_swap_buffers();
        inmp441_normalize_audio_buffer();

        audio_feed_samples_24bit(audio, inmp441_get_audio_buffer());
        audio_envelope(audio);
        audio_fft(audio);

        visualizer_map_frequency_bins_to_pixels( // TODO: Make this!
            audio_get_frequency_bins(audio),
            audio_get_frequency_bin_count(audio), ws2812_get_pixel_buffer(),
            ws2812_get_pixel_count());

        ws2812_swap_buffers();
    }

    return EXIT_SUCCESS;
}
