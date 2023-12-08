#include "audio.h"
#include "color.h"
#include "inmp441.h"
#include "swapchain.h"
#include "ws2812.h"

#include <pico/stdlib.h>
#include <pico/types.h>
#include <stdio.h>
#include <stdlib.h>

#define AUDIO_SAMPLE_COUNT 256
#define LED_COUNT 300

#define MIC_SCK_PIN 10
#define MIC_WS_PIN 11
#define MIC_DATA_PIN 12

#define LED_DATA_PIN 13

static color_grba_t magnitude_to_color(float magnitude) {
    if (magnitude < 0.f) {
        magnitude = 0.f;
    }

    if (magnitude > 1.f) {
        magnitude = 1.f;
    }

    return color_grba_from_rgb(0xFF, 0xFF, 0xFF);
}

static void visualizer_map_frequency_bins_to_pixels(const float *frequency_bins,
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
                color_grba_add(
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
    audio_t audio;
    swapchain_t audio_swapchain;
    swapchain_t led_swapchain;

    stdio_usb_init();

    if (!swapchain_init(&audio_swapchain,
                        inmp441_required_buffer_size(AUDIO_SAMPLE_COUNT))) {
        printf("Could not initialize audio swapchain\n");
        return EXIT_FAILURE;
    }

    if (!swapchain_init(&led_swapchain,
                        ws2812_required_buffer_size(LED_COUNT))) {
        printf("Could not initialize LED swapchain\n");
        return EXIT_FAILURE;
    }

    if (!inmp441_init(&audio_swapchain, AUDIO_SAMPLE_COUNT, MIC_SCK_PIN,
                      MIC_WS_PIN, MIC_DATA_PIN)) {
        printf("Could not initialize INMP441 driver");
        return EXIT_FAILURE;
    }

    if (!ws2812_init(&led_swapchain, LED_COUNT, LED_DATA_PIN)) {
        printf("Could not initialize WS2812 driver");
        return EXIT_FAILURE;
    }

    if (!audio_init(&audio, AUDIO_SAMPLE_COUNT)) {
        printf("Could not initialize audio");
        return EXIT_FAILURE;
    }

    inmp441_start_sampling();
    ws2812_start_transmission();

    while (true) {
        swapchain_consumer_swap(&audio_swapchain);

        audio_feed_inmp441(&audio, swapchain_consumer_buffer(&audio_swapchain));
        audio_envelope(&audio);
        audio_gain(&audio, 1.5f);
        audio_fft(&audio);

        visualizer_map_frequency_bins_to_pixels(
            audio_get_frequency_bins(&audio),
            audio_get_frequency_bin_count(&audio),
            swapchain_producer_buffer(&led_swapchain),
            ws2812_get_pixel_count());

        swapchain_producer_swap(&led_swapchain);
    }

    return EXIT_SUCCESS;
}
