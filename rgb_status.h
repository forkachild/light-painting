#pragma once

#include "pico/types.h"

typedef struct RGBStatus RGBStatus;

void rgb_status_init(RGBStatus **pp_status);
void rgb_status_set_color_blocking(RGBStatus *p_status, uint32_t color);
void rgb_status_set_color_rgb_blocking(RGBStatus *p_status, uint8_t red,
                                       uint8_t green, uint8_t blue);
void rgb_status_deinit(RGBStatus **pp_status);