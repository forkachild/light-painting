#pragma once

#include "pico/types.h"
#include <stdlib.h>

#define RGB2CanvasColor(r, g, b)                                               \
    GRBAColor {                                                                \
        .channels = {                                                          \
            .red = r,                                                          \
            .green = g,                                                        \
            .blue = b,                                                         \
        }                                                                      \
    }

typedef union {
    struct {
        uint8_t green;
        uint8_t blue;
        uint8_t red;
        uint8_t alpha;
    } channels;
    uint32_t value;
} CanvasColor;

typedef struct Canvas Canvas;

void canvas_init(Canvas **pp_canvas, uint count);
void canvas_clear(Canvas *p_canvas, CanvasColor color);
void canvas_point(Canvas *p_canvas, uint pos, CanvasColor color);
void canvas_line(Canvas *p_canvas, uint start, uint end, CanvasColor color);
void canvas_line_gradient(Canvas *p_canvas, uint start, uint end,
                          const CanvasColor *p_color_array,
                          uint colorArrayCount);
void canvas_line_rainbow(Canvas *p_canvas, uint start, uint end, float phase);
const uint32_t *canvas_get_grba_buffer(Canvas *p_canvas);
void canvas_deinit(Canvas **pp_canvas);