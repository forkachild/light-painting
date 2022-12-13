#include "canvas/canvas.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/types.h"
#include "status/status.h"
#include "ws2812_pio/driver.h"
#include <math.h>
#include <memory.h>
#include <stdio.h>

#include "fft/fft.h"
#include "inmp441_pio/driver.h"

#define N 64

#define TEST_FREQ_1 3
#define TEST_FREQ_2 5

#define TEST_COUNT 1000

static ComplexType SAMPLES[N];

int main() {
    stdio_init_all();

    RealType angle_per_sample = 2 * M_PI / N;

    for (uint i = 0; i < N; i++)
        SAMPLES[i] = sin(angle_per_sample * TEST_FREQ_1 * i) +
                     sin(angle_per_sample * TEST_FREQ_2 * i);

    uint *reversed_indices = cache_reversed_indices(N);
    ComplexType *twiddles = cache_twiddles(N);
    ComplexType *input = malloc(N * sizeof(ComplexType));

    sleep_ms(5000);

    for (uint i = 0; i < TEST_COUNT; i++) {
        memcpy(input, SAMPLES, N * sizeof(ComplexType));
        absolute_time_t start_time = get_absolute_time();
        fft_dit_radix2(input, N, reversed_indices, twiddles);
        absolute_time_t end_time = get_absolute_time();
        printf("Time %llu\n",
               to_us_since_boot(end_time) - to_us_since_boot(start_time));
    }

    // sleep_ms(5000);

    // fft_dit(SAMPLES, N, reversed_indices, twiddles);

    // sleep_ms(5000);

    // for (uint i = 0; i < N; i++)
    //     printf("%dHz: %f\n", i, 2 * cabs(SAMPLES[reversed_indices[i]]) / N);

    free(input);
    free(twiddles);
    free(reversed_indices);

    return EXIT_SUCCESS;
}

// #define LED_COUNT 300
// #define LINE_LENGTH 40

// #define WS2812_PIN 7
// // #define PROFILE

// int main()
// {
//     stdio_init_all();

//     status_init();
//     status_show(true);

//     WS2812PioDriver *driver;
//     ws2812_pio_driver_init(&driver, WS2812_PIN, LED_COUNT);

//     Canvas *canvas;
//     canvas_init(&canvas, LED_COUNT);

//     const CanvasColor black = {
//         .channels = {
//             .red = 0,
//             .green = 0,
//             .blue = 0,
//             .alpha = 0,
//         },
//     };

//     const CanvasColor red = {
//         .channels = {
//             .red = 0xFF,
//             .green = 0,
//             .blue = 0,
//             .alpha = 0,
//         },
//     };

//     uint value = 0;
//     uint dir = 1;

//     const CanvasColor colors[] = {
//         {
//             .value = 0xFF000000,
//         },
//         {
//             .value = 0x00FF0000,
//         },
//     };

//     while (true)
//     {
//         // canvas_clear(canvas, black);
//         // canvas_line_gradient(canvas, 0, LED_COUNT, colors,
//         count_of(colors));

//         canvas_line(canvas, value, value + LINE_LENGTH, red);
// #ifdef PROFILE
//         absolute_time_t xferStartTime = get_absolute_time();
// #endif
//         ws2812_pio_driver_submit_buffer_blocking(driver,
//         canvas_get_grba_buffer(canvas));
// #ifdef PROFILE
//         absolute_time_t xferEndTime = get_absolute_time();

//         int64_t timeTaken = absolute_time_diff_us(xferStartTime,
//         xferEndTime); printf("%lldus\n", timeTaken);
// #endif
//         canvas_line(canvas, value, value + LINE_LENGTH, black);

//         value += dir;

//         if (value == 0 || value == LED_COUNT - LINE_LENGTH)
//         {
//             dir *= -1;
//         }

//         sleep_us(300);
//     }

//     status_show(true);

//     canvas_deinit(&canvas);
//     ws2812_pio_driver_deinit(&driver);
//     status_deinit();

//     return EXIT_SUCCESS;
// }