#pragma once

#include "pico/stdlib.h"
#include "pico/types.h"
#include "types.h"
#include <math.h>
#include <memory.h>
#include <stdlib.h>

#define RGB2CanvasColor(r, g, b)                                               \
    GRBAColor {                                                                \
        .channels = {                                                          \
            .red = r,                                                          \
            .green = g,                                                        \
            .blue = b,                                                         \
        }                                                                      \
    }

typedef struct Canvas Canvas;
typedef union CanvasColor CanvasColor;

union CanvasColor {
    struct {
        uint8_t alpha;
        uint8_t blue;
        uint8_t red;
        uint8_t green;
    } channels;
    uint32_t value;
};

struct Canvas {
    uint count;
    uint32_t *buffer;
};

Result canvas_init(Canvas *canvas, uint count);
void canvas_clear(Canvas *canvas, CanvasColor color);
void canvas_point(Canvas *canvas, uint pos, CanvasColor color);
void canvas_line(Canvas *canvas, uint start, uint end, CanvasColor color);
void canvas_line_gradient(Canvas *canvas, uint start, uint end,
                          const CanvasColor *color_array, uint color_count);
void canvas_line_rainbow(Canvas *canvas, uint start, uint end, float phase);
uint32_t *canvas_get_grba_buffer(Canvas *canvas);
Result canvas_deinit(Canvas *canvas);
CanvasColor hsv_to_color(float hue, float saturation, float value);