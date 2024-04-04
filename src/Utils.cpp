#include <Utils.h>

void Utils::PrintBuffer(uint8_t buffer[], size_t len) {
    for (size_t i = 0; i < len; i++) {
        printk(" 0x%.2x", buffer[i]);
    }
    printk("\n");
};

void Utils::ShiftRightLogicalBuffer(uint8_t buffer[], size_t len){
    // need to shift the bits of the rx buffer (srl by 1 bit):
    // starting from back of rx buffer:
    for (size_t i = len - 1; i > 0; i--) {
        buffer[i] >>= 1;
        // overflow from the byte ahead of current:
        buffer[i] |= (buffer[i - 1] & 0x01) << 7;
    }
    // correct first byte
    buffer[0] >>= 1;
};

int Utils::get_whole_number_from_float(float32_t value) {
    return static_cast<int>(value);
}

int Utils::get_decimal_number_from_float(float32_t value, int precision) {
    float32_t decimal = value - get_whole_number_from_float(value);
    int multiplier = pow(10, precision);
    return static_cast<int>(decimal * multiplier);
}