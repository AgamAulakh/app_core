#pragma once
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>
#include <zephyr/logging/log.h>
#include "Data.h"
#include <stdio.h>
#include <string>
#include "core/MessageQueue.h"

#define LCD_RESULTS_MSG_Q_DEPTH 10
#define RESULT_READ_TIMEOUT_TICKS K_MSEC(100)

class LCD {
public:
    static Result most_recent_result;
    static MessageQueue<Result, LCD_RESULTS_MSG_Q_DEPTH> result_queue;
    static std::string delta_str;
    static std::string theta_str;
    static std::string alpha_str;
    static std::string beta_str;

    static bool SendResult(Result to_write) {
        if (result_queue.push(to_write) == false) {
            return false;
        }
        return true;
    };

    static void lcd_init();
    static void display_init();
    static void display_testing();
    static void display_processing();
    static void display_complete();
    static void display_cancel();
    static void display_demo_mode();
};