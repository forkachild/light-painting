#pragma once

#include <stdlib.h>
#include <stdint.h>

#define ARGB(a, r, g, b) ((a << 24) | (r << 16) | (g << 8) | b)

typedef struct WS2812Canvas WS2812Canvas;

void ws2812_canvas_init(WS2812Canvas **ppCanvas, size_t ledCount);
const uint32_t *ws2812_canvas_get_argb_buffer(WS2812Canvas *pCanvas);
void ws2812_canvas_clear(WS2812Canvas *pCanvas, uint32_t color);
void ws2812_canvas_draw_line(WS2812Canvas *pCanvas, size_t start, size_t end, uint32_t color);
void ws2812_canvas_deinit(WS2812Canvas **ppCanvas);