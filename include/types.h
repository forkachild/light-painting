#pragma once

typedef enum {
    RESULT_ALL_OK,
    RESULT_NOT_INIT,
    RESULT_ALREADY_INIT,
    RESULT_DMA_ERR,
    RESULT_PIO_ERR,
    RESULT_UNKNOWN_ERR,
} Result;