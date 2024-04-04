#pragma once

#include <cstdint>
#include <arm_math.h>
#include <zephyr/sys/printk.h>

namespace Utils {
    void PrintBuffer(uint8_t buffer[], size_t len);
    void ShiftRightLogicalBuffer(uint8_t buffer[], size_t len);
    int get_whole_number_from_float(float32_t value);
    int get_decimal_number_from_float(float32_t value, int precision);
};