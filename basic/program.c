#include <stdio.h>
#include "driver.h"
#include "canvas.h"
#include "status.h"
#include "pico/time.h"

#define LED_COUNT 300

int main()
{
    stdio_init_all();

    ws2812_status_init();

    WS2812Driver *driver;
    ws2812_driver_init(&driver, LED_COUNT);

    WS2812Canvas *canvas;
    ws2812_canvas_init(&canvas, LED_COUNT);

    ws2812_status_show(true);

    while (true)
    {
        ws2812_canvas_clear(canvas, 0xFFFF0000);
        ws2812_driver_submit_argb_buffer_blocking(driver, ws2812_canvas_get_argb_buffer(canvas));
        sleep_ms(1000);
        ws2812_canvas_clear(canvas, 0xFF00FF00);
        ws2812_driver_submit_argb_buffer_blocking(driver, ws2812_canvas_get_argb_buffer(canvas));
        sleep_ms(1000);
        ws2812_canvas_clear(canvas, 0xFF0000FF);
        ws2812_driver_submit_argb_buffer_blocking(driver, ws2812_canvas_get_argb_buffer(canvas));
        sleep_ms(1000);
    }

    ws2812_canvas_deinit(&canvas);
    ws2812_driver_deinit(&driver);
    ws2812_status_deinit();

    return 0;
}