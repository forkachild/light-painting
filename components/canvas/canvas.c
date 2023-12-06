#include "canvas.h"

#include "pico/stdlib.h"
#include "pico/types.h"
#include <math.h>
#include <memory.h>
#include <stdlib.h>

static GRBAColor hsv_to_grba(float hue, float sat, float val) {
    float hh, p, q, t, ff, r, g, b;
    int i;
    GRBAColor out;

    if (sat <= 0.f) { // < is bogus, just shuts up warnings
        r = val;
        g = val;
        b = val;
    } else {
        hh = hue;
        if (hh >= 360.f)
            hh = 0.f;
        hh /= 60.f;

        i = (int)hh;
        ff = hh - i;

        p = val * (1.f - sat);
        q = val * (1.f - (sat * ff));
        t = val * (1.f - (sat * (1.f - ff)));

        switch (i) {
        case 0:
            r = val;
            g = t;
            b = p;
            break;
        case 1:
            r = q;
            g = val;
            b = p;
            break;
        case 2:
            r = p;
            g = val;
            b = t;
            break;

        case 3:
            r = p;
            g = q;
            b = val;
            break;
        case 4:
            r = t;
            g = p;
            b = val;
            break;
        case 5:
        default:
            r = val;
            g = p;
            b = q;
            break;
        }
    }

    out.channels.red = (uint8_t)(r * 255);
    out.channels.green = (uint8_t)(g * 255);
    out.channels.blue = (uint8_t)(b * 255);

    return out;
}

int canvas_init(Canvas *canvas, uint count) {
    canvas->count = count;
    canvas->buffer = (uint32_t *)malloc(count * sizeof(uint32_t));

    if (!canvas->buffer)
        return -1;

    return 0;
}

void canvas_clear(Canvas *canvas, GRBAColor color) {
    uint i;

    for (i = 0; i < canvas->count; i++)
        canvas->buffer[i] = color.value;
}

void canvas_point(Canvas *canvas, uint pos, GRBAColor color) {
    if (pos >= canvas->count)
        return;

    canvas->buffer[pos] = color.value;
}

void canvas_line(Canvas *canvas, uint start, uint end, GRBAColor color) {
    if (end > canvas->count)
        end = canvas->count;

    if (start >= end)
        return;

    for (uint i = start; i < end; i++)
        canvas->buffer[i] = color.value;
}

void canvas_line_gradient(Canvas *canvas, uint start, uint end,
                          const GRBAColor *color_array, uint color_count) {
    uint i, index, next_index;
    GRBAColor color;
    float progress, f;

    if (end > canvas->count)
        end = canvas->count;

    if (start >= end)
        return;

    for (i = start; i < end; i++) {
        progress = (float)(i - start) / color_count;
        index = (uint)progress;
        next_index = (index + 1) % color_count;

        f = progress - index;

        color.channels.red =
            (uint8_t)(color_array[index].channels.red * (1.f - f)) +
            (uint8_t)(color_array[next_index].channels.red * f);
        color.channels.green =
            (uint8_t)(color_array[index].channels.green * (1.f - f)) +
            (uint8_t)(color_array[next_index].channels.green * f);
        color.channels.blue =
            (uint8_t)(color_array[index].channels.blue * (1.f - f)) +
            (uint8_t)(color_array[next_index].channels.blue * f);

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

    for (uint i = start; i < end; i++) {
        degrees = degree_delta * i;
        degrees = degrees + phase;
        degrees = fmodf(degrees, 360.f);
        canvas->buffer[i] = hsv_to_grba(degrees, 1.0f, 1.0f).value;
    }
}

uint32_t *canvas_get_grba_buffer(Canvas *canvas) { return canvas->buffer; }

void canvas_deinit(Canvas *canvas) {
    free(canvas->buffer);
}
