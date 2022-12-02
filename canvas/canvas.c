#include "canvas.h"

#include <stdlib.h>
#include <memory.h>
#include "pico/stdlib.h"

static CanvasColor hsv_to_canvas_color(float hue, float sat, float val);

struct Canvas
{
    uint count;
    uint32_t *pBuffer;
};

void canvas_init(Canvas **ppCanvas, uint count)
{
    Canvas *canvas = malloc(sizeof(Canvas));

    canvas->count = count;
    canvas->pBuffer = malloc(count * sizeof(uint32_t));

    *ppCanvas = canvas;
}

void canvas_clear(Canvas *pCanvas, CanvasColor color)
{
    for (int i = 0; i < pCanvas->count; i++)
    {
        pCanvas->pBuffer[i] = color.value;
    }
}

void canvas_line(
    Canvas *pCanvas,
    uint start,
    uint end,
    CanvasColor color)
{
    if (end > pCanvas->count)
    {
        end = pCanvas->count;
    }

    if (start >= end)
    {
        return;
    }

    for (int i = start; i < end; i++)
    {
        pCanvas->pBuffer[i] = color.value;
    }
}

void canvas_line_gradient(
    Canvas *pCanvas,
    uint start,
    uint end,
    CanvasColor *pColorArray,
    uint colorArrayCount)
{
    if (end > pCanvas->count)
    {
        end = pCanvas->count;
    }

    if (start >= end)
    {
        return;
    }

    float fDelta = 1.f / colorArrayCount;

    for (int i = start; i < end; i++)
    {
        float position = 1.f * (i - start) * (colorArrayCount - 1) / (end - start);
        int index = (int)position;
        CanvasColor startColor = pColorArray[index];
        CanvasColor endColor = pColorArray[index + 1];

        float fraction = position - index;

        // pCanvas->pBuffer[i] =
    }

    if (fDelta < 1.f)
    {
        return;
    }
}

void canvas_line_rainbow(
    Canvas *pCanvas,
    uint start,
    uint end,
    float phase)
{
}

const inline uint32_t *canvas_get_grba_buffer(Canvas *pCanvas)
{
    return pCanvas->pBuffer;
}

void canvas_deinit(Canvas **ppCanvas)
{
    Canvas *canvas = *ppCanvas;

    free(canvas->pBuffer);
    free(canvas);

    *ppCanvas = NULL;
}

CanvasColor hsb_to_canvas_color(float hue, float saturation, float brightness)
{
    float r, g, b;

    if (saturation == 0)
    {
        r = brightness;
        g = brightness;
        b = brightness;
    }
    else
    {
        if (hue == 360)
            hue = 0;
    }
}