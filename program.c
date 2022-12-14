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

#define AUDIO_SAMPLES 64
#define LED_COUNT 300

#define DATA_PIN 10
#define SCK_PIN 11
#define WS_PIN 12
#define LR_PIN 13
#define LED_PIN 11

#define AUDIO_INPUT_MAX_AMP ((1U << 24) - 1)

int main() {
    INMP441PioDriver *audio_driver = NULL;
    INMP441PioBuffer *audio_buffer = NULL;
    WS2812PioDriver *led_driver = NULL;
    Canvas *canvas = NULL;
    ComplexType *fft_samples = NULL;
    ComplexType *twiddles = NULL;
    uint *reversed_indices = NULL;
    // CanvasColor color = {.value = 0x000000FF};

    stdio_init_all();

    inmp441_pio_driver_init(&audio_driver, SCK_PIN, DATA_PIN, LR_PIN);
    if (!audio_driver) {
        printf("PIO Driver init failed\n");
        return EXIT_FAILURE;
    }

    inmp441_pio_buffer_init(&audio_buffer, AUDIO_SAMPLES);
    if (!audio_buffer) {
        printf("PIO Buffer init failed\n");
        return EXIT_FAILURE;
    }

    ws2812_pio_driver_init(&led_driver, LED_PIN, LED_COUNT);
    if (!led_driver) {
        printf("LED Driver init failed\n");
        return EXIT_FAILURE;
    }

    canvas_init(&canvas, LED_COUNT);
    if (!canvas) {
        printf("Canvas init failed\n");
        return EXIT_FAILURE;
    }

    fft_samples = malloc(AUDIO_SAMPLES * sizeof(ComplexType));
    if (!fft_samples) {
        printf("FFT Samples alloc failed\n");
        return EXIT_FAILURE;
    }

    twiddles = cache_twiddles(AUDIO_SAMPLES);
    if (!twiddles) {
        printf("Twiddles cache failed\n");
        return EXIT_FAILURE;
    }

    reversed_indices = cache_reversed_indices(AUDIO_SAMPLES);
    if (!reversed_indices) {
        printf("Reversed indices cache failed\n");
        return EXIT_FAILURE;
    }

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
            printf("%d ", audio_buffer_samples[i]);
        printf("\n");
        continue;

        for (uint i = 0; i < AUDIO_SAMPLES; i++)
            fft_samples[i] =
                (RealType)audio_buffer_samples[i] / AUDIO_INPUT_MAX_AMP;

        fft_radix2_dif(fft_samples, AUDIO_SAMPLES, twiddles);

        // Samples 1 to 30
        for (uint i = 1; i < (AUDIO_SAMPLES / 2) - 1; i++) {
            RealType normalized_sample =
                2 * cabs(fft_samples[reversed_indices[i]]) / AUDIO_SAMPLES;

            printf("%.1f ", normalized_sample);

            // color.channels.red = normalized_sample * 0xFF;
            // const uint start = i * 30;
            // canvas_line(canvas, start, start + 30, color);
        }
        printf("\n");

        // ws2812_pio_driver_submit_buffer_blocking(
        //     led_driver, canvas_get_grba_buffer(canvas));

#ifdef PROFILE
        end_time = get_absolute_time();
        printf("%llu\n",
               to_us_since_boot(end_time) - to_us_since_boot(start_time));
#endif
    }

    free(reversed_indices);
    free(twiddles);
    free(fft_samples);
    canvas_deinit(&canvas);
    ws2812_pio_driver_deinit(&led_driver);
    inmp441_pio_buffer_deinit(&audio_buffer);
    inmp441_pio_driver_deinit(&audio_driver);

    return EXIT_SUCCESS;
}