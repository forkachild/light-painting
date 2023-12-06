#pragma once

#include "pico/stdlib.h"
#include "pico/types.h"
#include <math.h>
#include <memory.h>
#include <stdlib.h>

#define RGB2GRBAColor(r, g, b)                                                 \
    GRBAColor {                                                                \
        .channels = {                                                          \
            .red = r,                                                          \
            .green = g,                                                        \
            .blue = b,                                                         \
        }                                                                      \
    }

typedef struct Canvas Canvas;
typedef union GRBAColor GRBAColor;
typedef struct HSVColor HSVColor;

union GRBAColor {
    struct {
        uint8_t alpha;
        uint8_t blue;
        uint8_t red;
        uint8_t green;
    } channels;
    uint32_t value;
};

struct HSVColor {
    float hue;
    float sat;
    float val;
};

struct Canvas {
    uint count;
    uint32_t *buffer;
};

int canvas_init(Canvas *canvas, uint count);
void canvas_clear(Canvas *canvas, GRBAColor color);
void canvas_point(Canvas *canvas, uint pos, GRBAColor color);
void canvas_line(Canvas *canvas, uint start, uint end, GRBAColor color);
void canvas_line_gradient(Canvas *canvas, uint start, uint end,
                          const GRBAColor *color_array, uint color_count);
void canvas_line_rainbow(Canvas *canvas, uint start, uint end, float phase);
uint32_t *canvas_get_grba_buffer(Canvas *canvas);
void canvas_deinit(Canvas *canvas);
