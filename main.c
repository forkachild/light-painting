#include "canvas.h"
#include "fft.h"
#include "status.h"
#include "swapchain.h"
#include "inmp441.h"
#include "ws2812.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/sync.h"
#include "pico/time.h"
#include "pico/types.h"
#include <complex.h>
#include <stdio.h>
#include <stdlib.h>

#define SAMPLE_COUNT 64
#define LED_COUNT 300

#define SPECTRUM_COVERAGE 0.2f
#define ATTENUATE_FACTOR 4
#define OUTPUT_SCALE 5.f

#define SCK_PIN 10
#define WS_PIN 11
#define DATA_PIN 12
#define LED_PIN 13
#define LR_PIN 14

#define AUDIO_INPUT_MAX_AMP (((int32_t)1 << 23) - 1)
#define FREQ_BIN_COUNT (SAMPLE_COUNT / 2)

static float complex *samples = NULL;
static float complex *twiddles = NULL;
static float *envelope = NULL;
static uint *reversed_indices = NULL;
static Canvas canvas;

const float sample_per_led =
    (SPECTRUM_COVERAGE * (FREQ_BIN_COUNT - 1)) / LED_COUNT;

static inline float get_freq_at(uint i) {
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

static inline GRBAColor get_color_at(uint i) {
    float exact_index = sample_per_led * i;
    uint sample_index = (uint)exact_index;
    float frac = exact_index - sample_index;

    // float freq_bin = get_freq_at(sample_index + 1);
    float freq_bin = (get_freq_at(sample_index + 1) * (1.f - frac)) +
                     (get_freq_at(sample_index + 2) * frac);
    freq_bin *= OUTPUT_SCALE;

    if (freq_bin > 1.f)
        freq_bin = 1.f;

    uint8_t value = freq_bin * 0xFF;
    return hsv_to_rgb(value, 0xFF, value);
}

static inline void setup_gpio() {
    gpio_init(LR_PIN);
    gpio_set_dir(LR_PIN, true);
    gpio_set_pulls(LR_PIN, false, true);
}

static inline void setup_drivers() {
    if (inmp441_init(SAMPLE_COUNT, SCK_PIN, WS_PIN, DATA_PIN) != 0) {
        printf("Audio init failed\n");
    }

    if (ws2812_init(LED_COUNT, LED_PIN) != 0) {
        printf("LED Driver init failed\n");
    }

    if (canvas_init(&canvas, LED_COUNT) != 0) {
        printf("Canvas init failed\n");
    }
}

static inline void cache_envelope() {
    envelope = malloc(SAMPLE_COUNT * sizeof(float));

    double angle_per_sample = M_PI / SAMPLE_COUNT;
    for (uint i = 0; i < SAMPLE_COUNT; i++)
        envelope[i] = sin(angle_per_sample * i);
}

static inline void cache_twiddles() {
    twiddles = malloc(FREQ_BIN_COUNT * sizeof(float complex));
    fill_twiddles(twiddles, SAMPLE_COUNT);
}

static inline void cache_reversed_indices() {
    reversed_indices = malloc(SAMPLE_COUNT * sizeof(uint));
    fill_reversed_indices(reversed_indices, SAMPLE_COUNT);
}

static inline void setup_buffers() {
    samples = malloc(SAMPLE_COUNT * sizeof(float complex));

    cache_twiddles();
    cache_reversed_indices();
    cache_envelope();
}

static inline void setup() {
    set_sys_clock_khz(200000, true);
    setup_gpio();
    setup_drivers();
    setup_buffers();
}

int main() {
    uint32_t saved_irq;

    setup();

    swapchain_context_t *audio_swapchain = NULL; // inmp441_get_swapchain();
    swapchain_context_t *led_swapchain = NULL;   // ws2812_get_swapchain();

    inmp441_start_sampling();
    ws2812_start_transmission();

#ifdef PROFILE_FPS
    uint frames = 0;
    absolute_time_t last_frame_time = get_absolute_time();
#endif

    for (;;) {
        saved_irq = save_and_disable_interrupts();
        const int32_t *int_samples = swapchain_get_right_buffer(audio_swapchain);
        restore_interrupts(saved_irq);

        for (uint i = 0; i < SAMPLE_COUNT; i++) {
            int32_t sample = int_samples[i] & 0xFFFFFF00;
            samples[i] =
                (float)(sample >> ATTENUATE_FACTOR) / AUDIO_INPUT_MAX_AMP;
            samples[i] *= envelope[i];
        }

        saved_irq = save_and_disable_interrupts();
        swapchain_flip_right(audio_swapchain);
        restore_interrupts(saved_irq);

        fft_rad2_dif(samples, twiddles, SAMPLE_COUNT);

        for (uint i = 1; i < LED_COUNT; i++)
            canvas_point(&canvas, i, get_color_at(i));

        saved_irq = save_and_disable_interrupts();
        swapchain_flip_left(led_swapchain);
        uint32_t *pixel_buffer = swapchain_get_left_buffer(led_swapchain);
        restore_interrupts(saved_irq);

        memcpy(pixel_buffer,
               canvas_get_grba_buffer(&canvas), LED_COUNT * sizeof(uint32_t));

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

    return EXIT_SUCCESS;
}