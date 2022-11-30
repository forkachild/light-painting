#include "canvas.h"

#include <stdlib.h>
#include <memory.h>
#include "pico/stdlib.h"

struct WS2812Canvas
{
    size_t ledCount;
    uint32_t *pPixels;
};

void ws2812_canvas_init(WS2812Canvas **ppCanvas, size_t ledCount)
{
    WS2812Canvas *canvas = malloc(sizeof(WS2812Canvas));
    canvas->ledCount = ledCount;
    canvas->pPixels = (uint32_t *)malloc(ledCount * sizeof(uint32_t));
    *ppCanvas = canvas;
}

const uint32_t *ws2812_canvas_get_argb_buffer(WS2812Canvas *pCanvas)
{
    return pCanvas->pPixels;
}

void ws2812_canvas_clear(WS2812Canvas *pCanvas, uint32_t color)
{
    for (int i = 0; i < pCanvas->ledCount; i++)
    {
        pCanvas->pPixels[i] = color;
    }
}

void ws2812_canvas_draw_line(WS2812Canvas *pCanvas, size_t start, size_t end, uint32_t color)
{
    for (int i = start; i < end; i++)
    {
        pCanvas->pPixels[i] = color;
    }
}

void ws2812_canvas_deinit(WS2812Canvas **ppCanvas)
{
    WS2812Canvas *canvas = *ppCanvas;
    free(canvas->pPixels);
    free(canvas);
    *ppCanvas = NULL;
}