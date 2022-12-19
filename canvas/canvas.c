#include "canvas.h"

#include <math.h>
#include <memory.h>
#include <stdlib.h>

#include "pico/stdlib.h"

static CanvasColor hsv_to_color(float hue, float saturation, float value) {
    float C = saturation * value;
    float X = C * (1 - abs(fmodf(hue / 60.0, 2) - 1));
    float m = value - C;
    float r, g, b;

    if (hue >= 0 && hue < 60) {
        r = C, g = X, b = 0;
    } else if (hue >= 60 && hue < 120) {
        r = X, g = C, b = 0;
    } else if (hue >= 120 && hue < 180) {
        r = 0, g = C, b = X;
    } else if (hue >= 180 && hue < 240) {
        r = 0, g = X, b = C;
    } else if (hue >= 240 && hue < 300) {
        r = X, g = 0, b = C;
    } else {
        r = C, g = 0, b = X;
    }

    CanvasColor color;

    color.channels.red = (r + m) * 255;
    color.channels.green = (g + m) * 255;
    color.channels.blue = (b + m) * 255;

    return color;
}

struct Canvas {
    uint count;
    uint32_t *p_buffer;
};

void canvas_init(Canvas **pp_canvas, uint count) {
    Canvas *canvas;

    if (*pp_canvas)
        return;

    canvas = malloc(sizeof(Canvas));

    canvas->count = count;
    canvas->p_buffer = malloc(count * sizeof(uint32_t));

    *pp_canvas = canvas;
}

void canvas_clear(Canvas *p_canvas, CanvasColor color) {
    uint i;

    for (i = 0; i < p_canvas->count; i++) {
        p_canvas->p_buffer[i] = color.value;
    }
}

void canvas_line(Canvas *p_canvas, uint start, uint end, CanvasColor color) {
    if (start < 0) {
        start = 0;
    }

    if (end > p_canvas->count) {
        end = p_canvas->count;
    }

    if (start >= end)
        return;

    for (int i = start; i < end; i++)
        p_canvas->p_buffer[i] = color.value;
}

void canvas_line_gradient(Canvas *p_canvas, uint start, uint end,
                          const CanvasColor *p_color_array,
                          uint colorArrayCount) {
    uint i, index, next_index;
    CanvasColor color;
    float progress, fraction;

    if (end > p_canvas->count)
        end = p_canvas->count;

    if (start >= end)
        return;

    for (i = start; i < end; i++) {
        progress = (float)(i - start) / colorArrayCount;
        index = (uint)progress;
        next_index = (index + 1) % colorArrayCount;

        fraction = progress - index;

        color.channels.red =
            (uint8_t)(p_color_array[index].channels.red * (1.f - fraction)) +
            (uint8_t)(p_color_array[next_index].channels.red * fraction);
        color.channels.green =
            (uint8_t)(p_color_array[index].channels.green * (1.f - fraction)) +
            (uint8_t)(p_color_array[next_index].channels.green * fraction);
        color.channels.blue =
            (uint8_t)(p_color_array[index].channels.blue * (1.f - fraction)) +
            (uint8_t)(p_color_array[next_index].channels.blue * fraction);

        p_canvas->p_buffer[i] = color.value;
    }
}

void canvas_line_rainbow(Canvas *p_canvas, uint start, uint end, float phase) {
    float degrees, degree_delta;

    if (end > p_canvas->count)
        end = p_canvas->count;

    if (start >= end)
        return;

    degree_delta = 360.f / (end - start);

    for (int i = start; i < end; i++) {
        degrees = degree_delta * i;
        degrees = degrees + phase;
        degrees = fmodf(degrees, 360.f);
        p_canvas->p_buffer[i] = hsv_to_color(degrees, 1.0f, 1.0f).value;
    }
}

const inline uint32_t *canvas_get_grba_buffer(Canvas *p_canvas) {
    return p_canvas->p_buffer;
}

void canvas_deinit(Canvas **pp_canvas) {
    Canvas *canvas;

    if (!(canvas = *pp_canvas))
        return;

    free(canvas->p_buffer);
    free(canvas);

    *pp_canvas = NULL;
}
