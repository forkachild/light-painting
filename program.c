#include <stdio.h>
#include "pico/stdlib.h"
#include "ws2812_pio/driver.h"
#include "canvas/canvas.h"
#include "status/status.h"
#include "pico/time.h"

#define LED_COUNT 300
#define LINE_LENGTH (LED_COUNT / 4)

int main()
{
    stdio_init_all();

    status_init();
    status_show(true);

    WS2812PioDriver *driver;
    ws2812_pio_driver_init(&driver, LED_COUNT);

    Canvas *canvas;
    canvas_init(&canvas, LED_COUNT);

    CanvasColor red = {
        .channels = {
            .red = 0xFF,
            .green = 0,
            .blue = 0,
            .alpha = 0,
        },
    };
    CanvasColor black = {.value = 0x00000000};

    int position = 0;
    int dir = 1;

    while (true)
    {
        canvas_line(canvas, position, position + LINE_LENGTH, red);
        ws2812_pio_driver_submit_buffer_blocking(driver, canvas_get_grba_buffer(canvas));
        canvas_line(canvas, position, position + LINE_LENGTH, black);

        position += dir;

        if (position == 0 || position > LED_COUNT - LINE_LENGTH)
        {
            dir *= -1;
        }

        sleep_ms(25);
    }

    status_show(true);

    canvas_deinit(&canvas);
    ws2812_pio_driver_deinit(&driver);
    status_deinit();

    return EXIT_SUCCESS;
}