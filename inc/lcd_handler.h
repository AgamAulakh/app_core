#pragma once
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>
#include <zephyr/logging/log.h>

namespace LCD {
    void display_init();
    void display_testing();
    void display_complete(float *alphaPower, float *betaPower, float *deltaPower, float *thetaPower);
    void display_cancel();
};

void lcd_init();