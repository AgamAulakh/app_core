#pragma once

#include <cstdint>
#include <arm_math.h>
#include <zephyr/sys/printk.h>

namespace Utils {
    extern float32_t inputSignal[1024];
    
    extern float32_t channelOne[512];
    extern float32_t channelTwo[512];
    extern float32_t channelThree[512];
    extern float32_t channelFour[512];
    extern float32_t channelFive[512];
    extern float32_t channelSix[512];
    extern float32_t channelSeven[512];
    extern float32_t channelEight[512];
   
    void PrintBuffer(uint8_t buffer[], size_t len);
    void ShiftRightLogicalBuffer(uint8_t buffer[], size_t len);
};