#include "canvas.h"

#include <stdlib.h>
#include "pico/stdlib.h"

void ws2812_canvas_init(WS2812Canvas *canvas, size_t ledCount)
{
    canvas->ledCount = ledCount;
    canvas->pixels = (uint32_t *)malloc(ledCount * sizeof(uint32_t));
}

const uint32_t *ws2812_canvas_get_argb_buffer(WS2812Canvas *canvas)
{
    return canvas->pixels;
}

void ws2812_canvas_clear(WS2812Canvas *canvas, uint32_t color)
{
    for (int i = 0; i < canvas->ledCount; i++)
    {
        canvas->pixels[i] = color;
    }
}

void ws2812_canvas_draw_line(WS2812Canvas *canvas, size_t start, size_t end, uint32_t color)
{
    for (int i = start; i < end; i++)
    {
        canvas->pixels[i] = color;
    }
}

void ws2812_canvas_deinit(WS2812Canvas *canvas)
{
    free(canvas->pixels);
}