#include <complex.h>
#include <stdio.h>
#include <stdlib.h>

#include "canvas/canvas.h"
#include "fft/fft.h"
#include "inmp441_pio/driver.h"
#include "inmp441_pio/swapchain.h"
#include "pico/stdlib.h"
#include "pico/sync.h"
#include "pico/time.h"
#include "pico/types.h"
#include "rgb_status.h"
#include "status/status.h"
#include "ws2812_pio/driver.h"

#include "hardware/gpio.h"

#define AUDIO_SAMPLES 64
#define LED_COUNT 300

#define SCK_PIN 3
#define WS_PIN 4
#define LR_PIN 5
#define DATA_PIN 6
#define LED_PIN 7
#define LED_GND 8

#define AUDIO_INPUT_MAX_AMP ((1U << 24) - 1)

int main() {
    // RGBStatus *rgb_status = NULL;
    WS2812PioDriver *led_driver = NULL;
    Canvas *canvas = NULL;
    float complex *fft_samples = NULL;
    float complex *twiddles = NULL;
    uint *reversed_indices = NULL;
    CanvasColor color = {.value = 0x000000FF};

    stdio_init_all();

    // rgb_status_init(&rgb_status);
    // rgb_status_set_color_rgb_blocking(rgb_status, 0, 0, 0);

    gpio_init(LED_GND);
    gpio_set_dir(LED_GND, true);
    gpio_pull_down(LED_GND);

    inmp441_driver_init(AUDIO_SAMPLES, SCK_PIN, WS_PIN, DATA_PIN, LR_PIN);

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

    INMP441Swapchain *swapchain = inmp441_driver_get_swapchain();
    INMP441SwapchainNode *node = NULL;

    inmp441_driver_start_sampling();

    for (;;) {
        // Get a node from swapchain
        uint32_t saved_irq = save_and_disable_interrupts();
        node = swapchain_borrow_for_read(swapchain);
        restore_interrupts(saved_irq);

        const uint32_t *audio_buffer_samples = swapchain_node_get_ptr(node);

        for (uint i = 0; i < AUDIO_SAMPLES; i++)
            fft_samples[i] =
                (float)audio_buffer_samples[i] / AUDIO_INPUT_MAX_AMP;

        // Submit back the node to swapchain
        saved_irq = save_and_disable_interrupts();
        swapchain_return_after_read(swapchain, node);
        restore_interrupts(saved_irq);

        fft_dif_rad2(fft_samples, twiddles, AUDIO_SAMPLES);

        // for (uint i = 0; i < AUDIO_SAMPLES / 4; i++)
        //     printf("%.2f  ", 2.f * cabs(fft_samples[reversed_indices[i]]) /
        //                          AUDIO_SAMPLES);
        // printf("\n");
        // continue;

        // Samples 1 to 30
        for (uint i = 1; i < (AUDIO_SAMPLES / 2) - 1; i++) {
            float normalized_sample =
                2 * cabs(fft_samples[reversed_indices[i]]) / AUDIO_SAMPLES;

            color.channels.red = normalized_sample * 0xFF;
            const uint start = i * 30;
            canvas_line(canvas, start, start + 30, color);
        }

        ws2812_pio_driver_submit_buffer_blocking(
            led_driver, canvas_get_grba_buffer(canvas));
    }

    inmp441_driver_stop_sampling();

    free(reversed_indices);
    free(twiddles);
    free(fft_samples);
    canvas_deinit(&canvas);
    ws2812_pio_driver_deinit(&led_driver);
    // rgb_status_deinit(&rgb_status);
    inmp441_driver_deinit();

    return EXIT_SUCCESS;
}
