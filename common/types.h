#ifndef TYPES_H
#define TYPES_H

typedef enum {
    RESULT_OK,
    RESULT_NOT_INIT,
    RESULT_ALREADY_INIT,
    RESULT_DMA_ERR,
    RESULT_PIO_ERR,
    RESULT_MEM_ERR,
    RESULT_ARG_ERR,
    RESULT_UNKNOWN_ERR,
} Result;

/**
 * Input(Audio) -> InFilter(0) -> InFilter(1) -> ... -> FFT -> OutFilter(0) ->
 * OutFilter(1) -> ... -> Output(LED)
 */

#endif