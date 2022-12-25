#include "canvas.h"
#include "fft.h"
#include "hardware/gpio.h"
#include "inmp441_pio.h"
#include "pico/stdlib.h"
#include "pico/sync.h"
#include "pico/time.h"
#include "pico/types.h"
#include "status.h"
#include "swapchain.h"
#include "ws2812_pio.h"
#include <complex.h>
#include <stdio.h>
#include <stdlib.h>

#define AUDIO_SAMPLES 64
#define LED_COUNT 300

#define SCK_PIN 3
#define WS_PIN 4
#define LR_PIN 5
#define DATA_PIN 6
#define LED_PIN 11
#define LED_GND 12

#define AUDIO_INPUT_MAX_AMP ((1U << 24) - 1)

int main() {
    Canvas canvas;
    float complex *fft_samples = NULL;
    float complex *twiddles = NULL;
    uint *reversed_indices = NULL;
    CanvasColor color = {.value = 0x000000FF};
    uint32_t saved_irq;

    stdio_init_all();

    // Providing an extra ground pin for the LEDs
    gpio_init(LED_GND);
    gpio_set_dir(LED_GND, true);
    gpio_put(LED_GND, false);

    inmp441_init(AUDIO_SAMPLES, SCK_PIN, WS_PIN, DATA_PIN, LR_PIN);

    if (ws2812_init(LED_COUNT, LED_PIN) != RESULT_ALL_OK) {
        printf("LED Driver init failed\n");
        return EXIT_FAILURE;
    }

    if (canvas_init(&canvas, LED_COUNT) != RESULT_ALL_OK) {
        printf("Canvas init failed\n");
        return EXIT_FAILURE;
    }

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

    Swapchain *audio_swapchain = inmp441_get_swapchain();
    Swapchain *led_swapchain = ws2812_get_swapchain();
    SwapchainNode *node = NULL;

    inmp441_start_sampling();
    ws2812_start_transmission();

    for (;;) {
        saved_irq = save_and_disable_interrupts();
        node = swapchain_borrow_for_read(audio_swapchain);
        restore_interrupts(saved_irq);

        const uint32_t *audio_buffer_samples =
            swapchain_node_get_buffer_ptr(node);

        for (uint i = 0; i < AUDIO_SAMPLES; i++)
            fft_samples[i] =
                (float)audio_buffer_samples[i] / AUDIO_INPUT_MAX_AMP;

        saved_irq = save_and_disable_interrupts();
        swapchain_return_after_read(audio_swapchain, node);
        restore_interrupts(saved_irq);

        fft_dif_rad2(fft_samples, twiddles, AUDIO_SAMPLES);

        for (uint i = 1; i < (AUDIO_SAMPLES / 2) - 1; i++) {
            float normalized_sample =
                2 * cabs(fft_samples[reversed_indices[i]]) / AUDIO_SAMPLES;

            color.channels.red = normalized_sample * 0xFF;
            const uint start = i * 30;
            canvas_line(&canvas, start, start + 30, color);
        }

        saved_irq = save_and_disable_interrupts();
        node = swapchain_borrow_for_write(led_swapchain);
        restore_interrupts(saved_irq);

        memcpy(swapchain_node_get_buffer_ptr(node),
               canvas_get_grba_buffer(&canvas), LED_COUNT);

        saved_irq = save_and_disable_interrupts();
        swapchain_return_after_write(led_swapchain, node);
        restore_interrupts(saved_irq);
    }

    inmp441_stop_sampling();
    ws2812_stop_transmission();

    free(reversed_indices);
    free(twiddles);
    free(fft_samples);
    canvas_deinit(&canvas);
    ws2812_deinit();
    inmp441_deinit();

    return EXIT_SUCCESS;
}
