#include "canvas.h"

canvas_t canvas_from_buffer(uint32_t *pb, size_t count) {
    return (canvas_t){
        .pb = pb,
        .count = count,
    };
}

void canvas_clear(canvas_t *self, uint32_t color) {
    for (int i = 0; i < self->count; i++)
        self->pb[i] = color;
}

void canvas_draw_line(canvas_t *self, int start, int end, uint32_t color,
                      canvas_op op) {
    start = start < 0 ? 0 : start;
    end = end > self->count ? self->count : end;

    for (int i = start; i < end; i++) {
        switch (op) {
        case OP_ADD:
            self->pb[i] += color;
            break;

        case OP_REPLACE:
            self->pb[i] = color;
            break;

        case OP_NONE:
        default:
            break;
        }
    }
}

// void canvas_draw_glowing_point(canvas_t *self, uint32_t color, int center,
//                                int spread, float glow, canvas_op op) {
//     if (spread <= 0)
//         return;

//     int start = center - spread;
//     int end = center + spread;

//     start = start < 0 ? 0 : start;
//     end = end > self->count ? self->count : end;
// }