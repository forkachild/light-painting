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

#define SCK_PIN 10
#define WS_PIN 11
#define DATA_PIN 12
#define LED_PIN 13
#define LR_PIN 14

#define AUDIO_INPUT_MAX_AMP (((int32_t)1 << 23) - 1)

int main() {
    Canvas canvas;
    GRBAColor color = {.value = 0x000000FF};

    float complex *fft_samples = NULL;
    float complex *twiddles = NULL;
    uint *reversed_indices = NULL;
    uint32_t saved_irq;

    // stdio_init_all();

    gpio_init(LR_PIN);
    gpio_set_dir(LR_PIN, true);
    gpio_set_pulls(LR_PIN, false, true);

    if (inmp441_init(AUDIO_SAMPLES, SCK_PIN, WS_PIN, DATA_PIN) !=
        RESULT_ALL_OK) {
        printf("Audio init failed\n");
        return EXIT_FAILURE;
    }

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

    // uint8_t position = 0;
    const float sample_per_led = (float)AUDIO_SAMPLES / (2 * LED_COUNT);
    // int32_t *signed_samples = malloc(AUDIO_SAMPLES * sizeof(int32_t));
    // uint32_t max = 0;
    // float phase = 0.f;
    // GRBAColor gradient_colors[] = {
    //     {0},
    //     {
    //         .channels = {.red = 0xFF},
    //     },
    //     {0},
    // };

#ifdef PROFILE_FPS
    uint frames = 0;
    absolute_time_t last_frame_time = get_absolute_time();
#endif

    for (;;) {
        saved_irq = save_and_disable_interrupts();
        node = swapchain_borrow_for_read(audio_swapchain);
        restore_interrupts(saved_irq);

        const int32_t *samples = (int32_t *)swapchain_node_get_buffer_ptr(node);

        for (uint i = 0; i < AUDIO_SAMPLES; i++)
            fft_samples[i] = (float)(samples[i] >> 8) / AUDIO_INPUT_MAX_AMP;

        samples = NULL;

        saved_irq = save_and_disable_interrupts();
        swapchain_return_after_read(audio_swapchain, node);
        restore_interrupts(saved_irq);

        // for (uint i = 0; i < AUDIO_SAMPLES; i++) {
        //     if (audio_samples[i] > max) {
        //         printf("Max %d\n", audio_samples[i]);
        //         max = audio_samples[i];
        //     }
        // }

        // for (uint i = 0; i < AUDIO_SAMPLES; i++) {
        //     color.channels.red = creal(fft_samples[i]) * 0xFF;
        //     canvas_point(&canvas, i, color);
        // }

        // for (uint i = 0; i < AUDIO_SAMPLES; i++)
        //     printf("%.2f  ", creal(fft_samples[i]));
        // printf("\n");

        fft_dif_rad2(fft_samples, twiddles, AUDIO_SAMPLES);

        for (uint i = 1; i < LED_COUNT; i++) {
            uint fft_index = (uint)(sample_per_led * i);

            float normalized_sample =
                2.f * cabs(fft_samples[reversed_indices[fft_index]]) /
                AUDIO_SAMPLES;

            color.channels.red = (uint8_t)(normalized_sample * 0xFF);
            canvas_point(&canvas, i, color);
        }

        // color.channels.green = position;
        // canvas_clear(&canvas, color);

        // if (++position >= 0xFF)
        //     position = 0;

        // canvas_line_gradient(&canvas, 0, LED_COUNT, gradient_colors,
        //                      count_of(gradient_colors));
        // canvas_line_rainbow(&canvas, 0, LED_COUNT, phase);

        // phase += 1.f;
        // if (phase >= 360.f)
        //     phase = 0.f;

        saved_irq = save_and_disable_interrupts();
        node = swapchain_borrow_for_write(led_swapchain);
        restore_interrupts(saved_irq);

        memcpy(swapchain_node_get_buffer_ptr(node),
               canvas_get_grba_buffer(&canvas), LED_COUNT * sizeof(uint32_t));

        saved_irq = save_and_disable_interrupts();
        swapchain_return_after_write(led_swapchain, node);
        restore_interrupts(saved_irq);

#ifdef PROFILE_FPS
        frames++;
        absolute_time_t time_now = get_absolute_time();
        if (absolute_time_diff_us(last_frame_time, time_now) > 1000000L) {
            printf("%d fps\n", frames);
            frames = 0;
            last_frame_time = time_now;
        }
#endif
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
