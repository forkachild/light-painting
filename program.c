#include <complex.h>
#include <stdio.h>
#include <stdlib.h>

#include "canvas/canvas.h"
#include "fft/fft.h"
#include "inmp441_pio/driver.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/types.h"
#include "status/status.h"
#include "ws2812_pio/driver.h"

#define AUDIO_SAMPLES 64
#define LED_COUNT 300

#define WS_PIN 7
#define CLK_PIN 8
#define DATA_PIN 9
#define LED_PIN 10

#define AUDIO_INPUT_MAX_AMP ((1 << 24) - 1)

int main() {
    INMP441PioDriver *audio_driver;
    INMP441PioBuffer *audio_buffer;
    WS2812PioDriver *led_driver;
    Canvas *canvas;
    ComplexType *fft_samples;
    ComplexType *twiddles;
    uint *reversed_indices;

    inmp441_pio_driver_init(&audio_driver, WS_PIN, DATA_PIN);
    inmp441_pio_buffer_init(&audio_buffer, AUDIO_SAMPLES);
    ws2812_pio_driver_init(&led_driver, LED_PIN, LED_COUNT);
    canvas_init(&canvas, LED_COUNT);

    fft_samples = malloc(AUDIO_SAMPLES * sizeof(ComplexType));
    twiddles = cache_reversed_indices(AUDIO_SAMPLES);
    reversed_indices = cache_reversed_indices(AUDIO_SAMPLES);

    for (;;) {
        // Receive the audio buffer
        inmp441_pio_driver_receive_blocking(audio_driver, audio_buffer);

        // Get the received buffer ptr
        const uint32_t *audio_buffer_samples =
            inmp441_pio_buffer_get_data_ptr(audio_buffer);

        for (uint i = 0; i < AUDIO_SAMPLES; i++)
            fft_samples[i] =
                (RealType)audio_buffer_samples[i] / AUDIO_INPUT_MAX_AMP;

        fft_dit_radix2(fft_samples, AUDIO_SAMPLES, reversed_indices, twiddles);

        for (uint i = 0; i < (AUDIO_SAMPLES / 2) - 2; i++)
            canvas_line(canvas, i * 30, i * 30 + 30,
                        (CanvasColor){
                            .channels = {
                                .blue = 0,
                                .green = 0,
                                .red = fft_samples[reversed_indices[i]] * 0xFF,
                            }});

        ws2812_pio_driver_submit_buffer_blocking(
            led_driver, canvas_get_grba_buffer(canvas));
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