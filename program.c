#include <stdio.h>
#include "pico/stdlib.h"
#include "ws2812_pio/driver.h"
#include "canvas/canvas.h"
#include "status/status.h"
#include "pico/time.h"

#define LED_COUNT 300
#define LINE_LENGTH 40

#define WS2812_PIN 7
// #define PROFILE

int main()
{
    stdio_init_all();

    status_init();
    status_show(true);

    WS2812PioDriver *driver;
    ws2812_pio_driver_init(&driver, WS2812_PIN, LED_COUNT);

    Canvas *canvas;
    canvas_init(&canvas, LED_COUNT);

    const CanvasColor black = {
        .channels = {
            .red = 0,
            .green = 0,
            .blue = 0,
            .alpha = 0,
        },
    };

    const CanvasColor red = {
        .channels = {
            .red = 0xFF,
            .green = 0,
            .blue = 0,
            .alpha = 0,
        },
    };

    uint8_t value = 0;
    uint8_t dir = 1;

    const CanvasColor colors[] = {
        {
            .value = 0xFF000000,
        },
        {
            .value = 0x00FF0000,
        },
    };

    while (true)
    {
        // canvas_clear(canvas, black);
        // canvas_line_gradient(canvas, 0, LED_COUNT, colors, count_of(colors));

        canvas_line(canvas, value, value + LINE_LENGTH, red);
#ifdef PROFILE
        absolute_time_t xferStartTime = get_absolute_time();
#endif
        ws2812_pio_driver_submit_buffer_blocking(driver, canvas_get_grba_buffer(canvas));
#ifdef PROFILE
        absolute_time_t xferEndTime = get_absolute_time();

        int64_t timeTaken = absolute_time_diff_us(xferStartTime, xferEndTime);
        printf("%lldus\n", timeTaken);
#endif
        canvas_line(canvas, value, value + LINE_LENGTH, black);

        value += dir;

        if (value <= 0 || value >= 255)
        {
            dir *= -1;
        }

        sleep_us(300);
    }

    status_show(true);

    canvas_deinit(&canvas);
    ws2812_pio_driver_deinit(&driver);
    status_deinit();

    return EXIT_SUCCESS;
}