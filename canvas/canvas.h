#pragma once

#include <stdlib.h>
#include "pico/types.h"

#define RGB2CanvasColor(r, g, b) \
    GRBAColor                    \
    {                            \
        .channels = {            \
            .red = r,            \
            .green = g,          \
            .blue = b,           \
        }                        \
    }

typedef union
{
    struct
    {
        uint8_t green;
        uint8_t blue;
        uint8_t red;
        uint8_t alpha;
    } channels;
    uint32_t value;
} CanvasColor;

typedef struct Canvas Canvas;

void canvas_init(Canvas **ppCanvas, uint count);
void canvas_clear(Canvas *pCanvas, CanvasColor color);
void canvas_line(
    Canvas *pCanvas,
    uint start,
    uint end,
    CanvasColor color);
void canvas_line_gradient(
    Canvas *pCanvas,
    uint start,
    uint end,
    const CanvasColor *pColorArray,
    uint colorArrayCount);
void canvas_line_rainbow(
    Canvas *pCanvas,
    uint start,
    uint end,
    float phase);
const uint32_t *canvas_get_grba_buffer(Canvas *pCanvas);
void canvas_deinit(Canvas **ppCanvas);