#ifndef CANVAS_H
#define CANVAS_H

#include <stdint.h>
#include <stdlib.h>

typedef enum { OP_NONE = 0, OP_ADD, OP_REPLACE } canvas_op;

typedef struct canvas {
    uint32_t *pb;
    int count;
} canvas_t;

canvas_t canvas_from_buffer(uint32_t *pb, size_t count);
void canvas_clear(canvas_t *canvas, uint32_t color);
void canvas_draw_line(canvas_t *canvas, int start, int end, uint32_t color,
                      canvas_op op);
void canvas_draw_glowing_point(canvas_t *canvas, uint32_t color, int center,
                               int spread, float glow, canvas_op op);
#endif