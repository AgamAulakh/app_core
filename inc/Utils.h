#pragma once

#include <cstdint>
#include <zephyr/sys/printk.h>

namespace Utils {
    void PrintBuffer(uint8_t buffer[], size_t len);
    void ShiftRightLogicalBuffer(uint8_t buffer[], size_t len);
};