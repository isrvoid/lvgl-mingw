#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int32_t x;
    int32_t y;
    int32_t encoder_pos;
    bool is_pressed;
    bool is_encoder_pressed;
} input_device_data_t;

input_device_data_t* input_device_data(void);
