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

#define SAMPLE_COUNT 64
#define LED_COUNT 300
#define AUDIO_INPUT_MAX_AMP (((int32_t)1 << 23) - 1)
#define FREQ_BIN_COUNT (SAMPLE_COUNT / 2)

#define SCK_PIN 10
#define WS_PIN 11
#define DATA_PIN 12
#define LED_PIN 13
#define LR_PIN 14

static float complex *samples = NULL;
static float complex *twiddles = NULL;
static uint *reversed_indices = NULL;
static Canvas canvas;

const float sample_per_led = (0.3f * FREQ_BIN_COUNT) / LED_COUNT;

static inline float get_abs_freq_bin(uint i) {
    return cabsf(samples[reversed_indices[i]]) / FREQ_BIN_COUNT;
}

static inline GRBAColor hsv_to_rgb(uint8_t h, uint8_t s, uint8_t v) {
    GRBAColor rgb;
    uint8_t region, remainder, p, q, t;

    if (s == 0) {
        rgb.channels.red = v;
        rgb.channels.green = v;
        rgb.channels.blue = v;
        return rgb;
    }

    region = h / 43;
    remainder = (h - (region * 43)) * 6;

    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region) {
    case 0:
        rgb.channels.red = v;
        rgb.channels.green = t;
        rgb.channels.blue = p;
        break;
    case 1:
        rgb.channels.red = q;
        rgb.channels.green = v;
        rgb.channels.blue = p;
        break;
    case 2:
        rgb.channels.red = p;
        rgb.channels.green = v;
        rgb.channels.blue = t;
        break;
    case 3:
        rgb.channels.red = p;
        rgb.channels.green = q;
        rgb.channels.blue = v;
        break;
    case 4:
        rgb.channels.red = t;
        rgb.channels.green = p;
        rgb.channels.blue = v;
        break;
    default:
        rgb.channels.red = v;
        rgb.channels.green = p;
        rgb.channels.blue = q;
        break;
    }

    return rgb;
}

static inline GRBAColor led_get_color(uint i) {
    float freq_bin = get_abs_freq_bin((sample_per_led * i) + 1) * 5.f;
    if (freq_bin > 1.f)
        freq_bin = 1.f;

    uint8_t value = freq_bin * 0xFF;

    return hsv_to_rgb(value, 0xFF, value);
}

int main() {
    uint32_t saved_irq;

    // stdio_init_all();

    gpio_init(LR_PIN);
    gpio_set_dir(LR_PIN, true);
    gpio_set_pulls(LR_PIN, false, true);

    if (inmp441_init(SAMPLE_COUNT, SCK_PIN, WS_PIN, DATA_PIN) !=
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

    samples = malloc(SAMPLE_COUNT * sizeof(float complex));
    if (!samples) {
        printf("FFT Samples alloc failed\n");
        return EXIT_FAILURE;
    }

    twiddles = malloc(FREQ_BIN_COUNT * sizeof(float complex));
    if (!twiddles) {
        printf("Twiddles alloc failed\n");
        return EXIT_FAILURE;
    }
    fill_twiddles(twiddles, SAMPLE_COUNT);

    reversed_indices = malloc(SAMPLE_COUNT * sizeof(uint));
    if (!reversed_indices) {
        printf("Reversed indices alloc failed\n");
        return EXIT_FAILURE;
    }
    fill_reversed_indices(reversed_indices, SAMPLE_COUNT);

    Swapchain *audio_swapchain = inmp441_get_swapchain();
    Swapchain *led_swapchain = ws2812_get_swapchain();
    SwapchainNode *node = NULL;

    inmp441_start_sampling();
    ws2812_start_transmission();

#ifdef PROFILE_FPS
    uint frames = 0;
    absolute_time_t last_frame_time = get_absolute_time();
#endif

    for (;;) {
        saved_irq = save_and_disable_interrupts();
        node = swapchain_borrow_for_read(audio_swapchain);
        restore_interrupts(saved_irq);

        const int32_t *int_samples =
            (int32_t *)swapchain_node_get_buffer_ptr(node);

        for (uint i = 0; i < SAMPLE_COUNT; i++)
            samples[i] = (float)(int_samples[i] >> 8) / AUDIO_INPUT_MAX_AMP;

        int_samples = NULL;

        saved_irq = save_and_disable_interrupts();
        swapchain_return_after_read(audio_swapchain, node);
        restore_interrupts(saved_irq);

        fft_dif_rad2(samples, twiddles, SAMPLE_COUNT);

        for (uint i = 1; i < LED_COUNT; i++)
            canvas_point(&canvas, i, led_get_color(i));

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
    free(samples);
    canvas_deinit(&canvas);
    ws2812_deinit();
    inmp441_deinit();

    return EXIT_SUCCESS;
}