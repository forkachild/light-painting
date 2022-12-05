#include "canvas.h"

#include <stdlib.h>
#include <memory.h>
#include <math.h>

#include "pico/stdlib.h"

static CanvasColor hsv_to_color(float hue, float sat, float val);

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
    if (start < 0)
    {
        start = 0;
    }

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
    const CanvasColor *pColorArray,
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

    CanvasColor color;

    for (int i = start; i < end; i++)
    {
        float progress = (float)(i - start) / colorArrayCount;
        uint index = (uint)progress;
        uint nextIndex = (index + 1) % colorArrayCount;

        float fraction = progress - index;

        color.channels.red = (uint8_t)(pColorArray[index].channels.red * (1.f - fraction)) + (uint8_t)(pColorArray[nextIndex].channels.red * fraction);
        color.channels.green = (uint8_t)(pColorArray[index].channels.green * (1.f - fraction)) + (uint8_t)(pColorArray[nextIndex].channels.green * fraction);
        color.channels.blue = (uint8_t)(pColorArray[index].channels.blue * (1.f - fraction)) + (uint8_t)(pColorArray[nextIndex].channels.blue * fraction);

        printf("Red: %02x, Green: %02X, Blue: %02X\n", color.channels.red, color.channels.green, color.channels.blue);
        pCanvas->pBuffer[i] = color.value;
    }
}

void canvas_line_rainbow(
    Canvas *pCanvas,
    uint start,
    uint end,
    float phase)
{
    if (end > pCanvas->count)
    {
        end = pCanvas->count;
    }

    if (start >= end)
    {
        return;
    }

    float fDegreeDelta = 360.f / (end - start);

    for (int i = start; i < end; i++)
    {
        float degrees = fDegreeDelta * i;
        degrees = degrees + phase;
        degrees = fmodf(degrees, 360.f);
        pCanvas->pBuffer[i] = hsv_to_color(degrees, 1.0f, 1.0f).value;
    }
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

CanvasColor hsv_to_color(float hue, float saturation, float value)
{
    float C = saturation * value;
    float X = C * (1 - abs(fmodf(hue / 60.0, 2) - 1));
    float m = value - C;
    float r, g, b;

    if (hue >= 0 && hue < 60)
    {
        r = C, g = X, b = 0;
    }
    else if (hue >= 60 && hue < 120)
    {
        r = X, g = C, b = 0;
    }
    else if (hue >= 120 && hue < 180)
    {
        r = 0, g = C, b = X;
    }
    else if (hue >= 180 && hue < 240)
    {
        r = 0, g = X, b = C;
    }
    else if (hue >= 240 && hue < 300)
    {
        r = X, g = 0, b = C;
    }
    else
    {
        r = C, g = 0, b = X;
    }

    CanvasColor color;

    color.channels.red = (r + m) * 255;
    color.channels.green = (g + m) * 255;
    color.channels.blue = (b + m) * 255;

    return color;
}