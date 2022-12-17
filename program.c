#include <complex.h>
#include <stdio.h>
#include <stdlib.h>

#include "canvas/canvas.h"
#include "fft/fft.h"
#include "inmp441_pio/buffer.h"
#include "inmp441_pio/driver.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/types.h"
#include "status/status.h"
#include "ws2812_pio/driver.h"

#include "hardware/gpio.h"

#define AUDIO_SAMPLES 64
#define LED_COUNT 300

#define LED_PIN 8
#define LED_GND 9
#define SCK_PIN 26
#define WS_PIN 27
#define LR_PIN 28
#define DATA_PIN 29

#define AUDIO_INPUT_MAX_AMP ((1U << 24) - 1)

int main() {
    INMP441PioDriver *audio_driver = NULL;
    INMP441PioBuffer *audio_buffer = NULL;
    // WS2812PioDriver *led_driver = NULL;
    // Canvas *canvas = NULL;
    float complex *fft_samples = NULL;
    float complex *twiddles = NULL;
    uint *reversed_indices = NULL;
    CanvasColor color = {.value = 0x000000FF};

    stdio_init_all();

    gpio_init(LED_GND);
    gpio_set_dir(LED_GND, true);
    gpio_pull_down(LED_GND);

    inmp441_pio_driver_init(&audio_driver, SCK_PIN, WS_PIN, DATA_PIN, LR_PIN);
    if (!audio_driver) {
        printf("PIO Driver init failed\n");
        return EXIT_FAILURE;
    }

    inmp441_pio_buffer_init(&audio_buffer, AUDIO_SAMPLES);
    if (!audio_buffer) {
        printf("PIO Buffer init failed\n");
        return EXIT_FAILURE;
    }

    // ws2812_pio_driver_init(&led_driver, LED_PIN, LED_COUNT);
    // if (!led_driver) {
    //     printf("LED Driver init failed\n");
    //     return EXIT_FAILURE;
    // }

    // canvas_init(&canvas, LED_COUNT);
    // if (!canvas) {
    //     printf("Canvas init failed\n");
    //     return EXIT_FAILURE;
    // }

    fft_samples = malloc(AUDIO_SAMPLES * sizeof(float complex));
    if (!fft_samples) {
        printf("FFT Samples alloc failed\n");
        return EXIT_FAILURE;
    }

    twiddles = malloc((AUDIO_SAMPLES / 2) * sizeof(float complex));
    if (!twiddles) {
        printf("Twiddles alloc failed\n");
        return EXIT_FAILURE;
    }
    fill_twiddles(twiddles, AUDIO_SAMPLES);

    reversed_indices = malloc(AUDIO_SAMPLES * sizeof(uint));
    if (!reversed_indices) {
        printf("Reversed indices alloc failed\n");
        return EXIT_FAILURE;
    }
    fill_reversed_indices(reversed_indices, AUDIO_SAMPLES);

#ifdef PROFILE
    absolute_time_t start_time, end_time;
#endif

    for (;;) {

#ifdef PROFILE
        start_time = get_absolute_time();
#endif
        // Receive the audio buffer
        inmp441_pio_driver_receive_blocking(audio_driver, audio_buffer);

        // Get the received buffer ptr
        const uint32_t *audio_buffer_samples =
            inmp441_pio_buffer_get_data_ptr(audio_buffer);

        for (uint i = 0; i < AUDIO_SAMPLES; i++)
            fft_samples[i] =
                (float)audio_buffer_samples[i] / AUDIO_INPUT_MAX_AMP;

        fft_dif_rad2(fft_samples, twiddles, AUDIO_SAMPLES);

        // Samples 1 to 30
        for (uint i = 1; i < (AUDIO_SAMPLES / 2) - 1; i++) {
            float normalized_sample =
                2 * cabs(fft_samples[reversed_indices[i]]) / AUDIO_SAMPLES;

            color.channels.red = normalized_sample * 0xFF;
            printf("%d ", color.channels.red);
            // const uint start = i * 30;
            // canvas_line(canvas, start, start + 30, color);
        }

        printf("\n");

#ifdef PROFILE
        end_time = get_absolute_time();
        printf("%llu\n",
               to_us_since_boot(end_time) - to_us_since_boot(start_time));
#endif
    }

    free(reversed_indices);
    free(twiddles);
    free(fft_samples);
    // canvas_deinit(&canvas);
    // ws2812_pio_driver_deinit(&led_driver);
    inmp441_pio_buffer_deinit(&audio_buffer);
    inmp441_pio_driver_deinit(&audio_driver);

    return EXIT_SUCCESS;
}
