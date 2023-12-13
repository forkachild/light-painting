#include "async.h"
#include "audio.h"
#include "color.h"
#include "i2s.h"
#include "neopixel.h"
#include "swapchain.h"
#include <malloc.h>
#include <math.h>

#include <pico/stdlib.h>
#include <pico/types.h>
#include <stdio.h>
#include <stdlib.h>

#define AUDIO_SAMPLE_RATE 48000
#define AUDIO_SAMPLE_COUNT 256
#define LED_COUNT 298

#define MIC_SCK_PIN 27
#define MIC_WS_PIN 28
#define MIC_DATA_PIN 29

#define LED_DATA_PIN 8

#define AUDIO_PEAK_AMPLITUDE ((uint32_t)0x00FFFFFF)
#define AUDIO_GAIN_MULTIPLIER 1000.f
#define AUDIO_SMOOTH_FACTOR .5f

static color_neopixel_t magnitude_to_color(float magnitude) {
    if (magnitude < 0.f)
        magnitude = 0.f;
    else if (magnitude > 1.f)
        magnitude = 1.f;

    return color_neopixel_from_hsv_f(magnitude * 240.f, 1.f, magnitude);
    // return color_neopixel_from_intensity(magnitude);
}

static void visualizer_map_frequency_bins_to_pixels(const float *frequency_bins,
                                                    size_t frequency_bin_count,
                                                    uint32_t *pixel_buffer,
                                                    size_t pixel_count) {
    if (frequency_bin_count != pixel_count) {
        float pitch = (float)(frequency_bin_count - 1) / pixel_count;

        for (size_t pixel = 0; pixel < pixel_count; pixel++) {
            float index = pixel * pitch;
            size_t index_i = (size_t)index;
            float index_f = index - index_i;

            pixel_buffer[pixel] =
                color_neopixel_add(
                    magnitude_to_color((1.f - index_f) *
                                       frequency_bins[index_i]),
                    magnitude_to_color(index_f * frequency_bins[index_i + 1]))
                    .value;
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
                        i2s_required_buffer_size(AUDIO_SAMPLE_COUNT))) {
        printf("Could not initialize audio swapchain\n");
        return EXIT_FAILURE;
    }

    if (!swapchain_init(&led_swapchain,
                        neopixel_required_buffer_size(LED_COUNT))) {
        printf("Could not initialize LED swapchain\n");
        return EXIT_FAILURE;
    }

    if (!i2s_init(&audio_swapchain, AUDIO_SAMPLE_COUNT, MIC_SCK_PIN, MIC_WS_PIN,
                  MIC_DATA_PIN)) {
        printf("Could not initialize i2s driver");
        return EXIT_FAILURE;
    }

    if (!neopixel_init(&led_swapchain, LED_COUNT, LED_DATA_PIN)) {
        printf("Could not initialize WS2812 driver");
        return EXIT_FAILURE;
    }

    if (!audio_init(&audio, AUDIO_SAMPLE_COUNT, AUDIO_SAMPLE_RATE, .8f)) {
        printf("Could not initialize audio");
        return EXIT_FAILURE;
    }

    i2s_start_sampling();
    neopixel_start_transmission();

    printf("Visualizing\n");

    while (true) {
        critical_section(swapchain_consumer_swap(&audio_swapchain));

        // Feed the stereo data
        audio_feed_i2s_stereo_24bit(&audio,
                                    swapchain_consumer_buffer(&audio_swapchain),
                                    AUDIO_PEAK_AMPLITUDE);

        // Process the signal
        // audio_normalize(&audio);
        audio_multiply(&audio, 50.f);
        audio_square_signed(&audio);
        // audio_scale_rms(&audio);
        audio_envelope(&audio);
        audio_fft(&audio);
        audio_square(&audio);
        // audio_fft_normalize(&audio);
        audio_multiply(&audio, 5.f);
        // audio_clip_below(&audio, 0.3f);
        audio_smooth_vertical(&audio, 0.7f);

        visualizer_map_frequency_bins_to_pixels(
            audio_fft_spectra(&audio), audio_fft_freq_spectra_count(&audio),
            swapchain_producer_buffer(&led_swapchain),
            neopixel_get_pixel_count());

        critical_section(swapchain_producer_swap(&led_swapchain));
    }

    neopixel_stop_transmission();
    i2s_stop_sampling();

    audio_deinit(&audio);
    neopixel_deinit();
    i2s_deinit();
    swapchain_deinit(&led_swapchain);
    swapchain_deinit(&audio_swapchain);

    return EXIT_SUCCESS;
}
