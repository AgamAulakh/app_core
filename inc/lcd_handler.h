#pragma once
#include <stdio.h>
#include <string>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>
#include <zephyr/logging/log.h>
#include "Data.h"
#include "Utils.h"
#include "core/MessageQueue.h"

#define LCD_RESULTS_MSG_Q_DEPTH 10
#define RESULT_READ_TIMEOUT_TICKS K_MSEC(100)

class LCD {
public:
    // static MessageQueue<Result, LCD_RESULTS_MSG_Q_DEPTH> result_queue;
    // static bool SendResult(Result to_write) {
    //     if (result_queue.push(to_write) == false) {
    //         printk("YIKES failed to push");
    //         return false;
    //     }
    //     return true;
    // };

    static char result_str[100];
    static Result most_recent_result;
    static void lcd_init();
    static void display_idle();
    static void display_init();
    static void display_testing();
    static void display_processing();
    static void display_complete();
    static void display_cancel();
    static void prepare_queue_for_new_result();
    static void display_demo_mode();
    static void update_most_recent_result(Result& to_update);
};