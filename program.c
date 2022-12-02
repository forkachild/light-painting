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

    status_show(false);

    Canvas *canvas;
    canvas_init(&canvas, LED_COUNT);

    status_show(true);

    CanvasColor red = {
        .value = 0xFF000000,
    };

    CanvasColor green = {
        .value = 0x00FF0000,
    };

    CanvasColor blue = {
        .value = 0x0000FF00,
    };

    while (true)
    {
        printf("I'm before red\n");
        status_show(false);
        canvas_clear(canvas, red);
        ws2812_pio_driver_submit_buffer_blocking(driver, canvas_get_grba_buffer(canvas));
        sleep_ms(500);
        printf("I'm before green\n");
        status_show(true);
        canvas_clear(canvas, green);
        ws2812_pio_driver_submit_buffer_blocking(driver, canvas_get_grba_buffer(canvas));
        sleep_ms(500);
        printf("I'm before blue\n");
        status_show(false);
        canvas_clear(canvas, blue);
        ws2812_pio_driver_submit_buffer_blocking(driver, canvas_get_grba_buffer(canvas));
        sleep_ms(500);
    }

    status_show(true);

    canvas_deinit(&canvas);
    ws2812_pio_driver_deinit(&driver);
    status_deinit();

    return 0;
}