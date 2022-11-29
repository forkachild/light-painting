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

    ws2812_status_blink_blocking(2, STATUS_DELAY_MEDIUM_MS);
    WS2812Driver *driver;
    ws2812_driver_init(&driver, LED_COUNT);

    ws2812_status_blink_blocking(3, STATUS_DELAY_FAST_MS);
    WS2812Canvas *canvas;
    ws2812_canvas_init(&canvas, LED_COUNT);

    ws2812_status_show(true);

    for (;;)
    {
        ws2812_canvas_clear(canvas, ARGB(0xFF, 0xFF, 0x00, 0x00));
        ws2812_driver_submit_argb_buffer_blocking(driver, ws2812_canvas_get_argb_buffer(canvas));
        sleep_ms(1000);
        ws2812_canvas_clear(canvas, ARGB(0xFF, 0x00, 0xFF, 0x00));
        ws2812_driver_submit_argb_buffer_blocking(driver, ws2812_canvas_get_argb_buffer(canvas));
        sleep_ms(1000);
        ws2812_canvas_clear(canvas, ARGB(0xFF, 0x00, 0x00, 0xFF));
        ws2812_driver_submit_argb_buffer_blocking(driver, ws2812_canvas_get_argb_buffer(canvas));
        sleep_ms(1000);
    }

    ws2812_canvas_deinit(&canvas);
    ws2812_driver_deinit(&driver);
    ws2812_status_deinit();

    return 0;
}