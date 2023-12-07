#include "visualization.h"
#include "audio.h"
#include "canvas.h"
#include "fft.h"
#include "inmp441.h"
#include "ws2812.h"

struct visualization_context {
    audio_context_t *audio;
    canvas_context_t *canvas;
};

int visualization_init(visualization_context_t **context,
                       size_t audio_sample_count, size_t pixel_count) {
    audio_context_t *audio;
    canvas_context_t *canvas;
    if (*context != VISUALIZATION_UNINIT)
        return -1;

    visualization_context_t *c =
        (visualization_context_t *)malloc(sizeof(visualization_context_t));

    if (c == NULL)
        return -1;

    if (!canvas_init(&canvas, pixel_count)) {
        free(c);
        return -1;
    }

    if (!audio_init(&audio, audio_sample_count)) {
        free(c);
        canvas_deinit(&canvas);
    }

    c->audio = audio;
    c->canvas = canvas;

    *context = c;

    return 1;
}

void canvas_render_and_clear(canvas_context_t *context) {}