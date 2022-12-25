#include "canvas.h"

#include "pico/stdlib.h"
#include "pico/types.h"
#include "types.h"
#include <math.h>
#include <memory.h>
#include <stdlib.h>

Result canvas_init(Canvas *canvas, uint count) {
    canvas->count = count;
    canvas->buffer = (uint32_t *)malloc(count * sizeof(uint32_t));

    return RESULT_ALL_OK;
}

void canvas_clear(Canvas *canvas, CanvasColor color) {
    uint i;

    for (i = 0; i < canvas->count; i++) {
        canvas->buffer[i] = color.value;
    }
}

void canvas_point(Canvas *canvas, uint pos, CanvasColor color) {
    if (pos >= canvas->count)
        return;

    canvas->buffer[pos] = color.value;
}

void canvas_line(Canvas *canvas, uint start, uint end, CanvasColor color) {
    if (end > canvas->count) {
        end = canvas->count;
    }

    if (start >= end)
        return;

    for (int i = start; i < end; i++)
        canvas->buffer[i] = color.value;
}

void canvas_line_gradient(Canvas *canvas, uint start, uint end,
                          const CanvasColor *color_array, uint color_count) {
    uint i, index, next_index;
    CanvasColor color;
    float progress, fraction;

    if (end > canvas->count)
        end = canvas->count;

    if (start >= end)
        return;

    for (i = start; i < end; i++) {
        progress = (float)(i - start) / color_count;
        index = (uint)progress;
        next_index = (index + 1) % color_count;

        fraction = progress - index;

        color.channels.red =
            (uint8_t)(color_array[index].channels.red * (1.f - fraction)) +
            (uint8_t)(color_array[next_index].channels.red * fraction);
        color.channels.green =
            (uint8_t)(color_array[index].channels.green * (1.f - fraction)) +
            (uint8_t)(color_array[next_index].channels.green * fraction);
        color.channels.blue =
            (uint8_t)(color_array[index].channels.blue * (1.f - fraction)) +
            (uint8_t)(color_array[next_index].channels.blue * fraction);

        canvas->buffer[i] = color.value;
    }
}

void canvas_line_rainbow(Canvas *canvas, uint start, uint end, float phase) {
    float degrees, degree_delta;

    if (end > canvas->count)
        end = canvas->count;

    if (start >= end)
        return;

    degree_delta = 360.f / (end - start);

    for (int i = start; i < end; i++) {
        degrees = degree_delta * i;
        degrees = degrees + phase;
        degrees = fmodf(degrees, 360.f);
        canvas->buffer[i] = hsv_to_color(degrees, 1.0f, 1.0f).value;
    }
}

uint32_t *canvas_get_grba_buffer(Canvas *canvas) { return canvas->buffer; }

Result canvas_deinit(Canvas *canvas) {
    free(canvas->buffer);
    return RESULT_ALL_OK;
}

CanvasColor hsv_to_color(float hue, float saturation, float value) {
    float C = saturation * value;
    float X = C * (1 - abs(fmodf(hue / 60.0, 2) - 1));
    float m = value - C;
    float r, g, b;
    CanvasColor color;

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

    color.channels.red = (r + m) * 255;
    color.channels.green = (g + m) * 255;
    color.channels.blue = (b + m) * 255;

    return color;
}