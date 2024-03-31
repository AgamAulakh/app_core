#pragma once
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>
#include <zephyr/smf.h>
#include <zephyr/logging/log.h>

#define THREAD_STATE_SIZE 1028 // arbitrary for now

#define DATA_ENABLE DT_ALIAS(databutton) // Button for initiating or cancelling test P1.00
#if !DT_NODE_HAS_STATUS(DATA_ENABLE, okay)
#error "Unsupported board: DATA_ENABLE devicetree alias is not defined"
#endif

/* List of events */
#define EVENT_BTN_PRESS BIT(0)

namespace TestButton {
    static const struct gpio_dt_spec data_enable_button = GPIO_DT_SPEC_GET_OR(DATA_ENABLE, gpios, {0});
    static struct gpio_callback button_cb_data;

    void init();
    void button_press(const struct device *dev, struct gpio_callback *cb, uint32_t pins);
};

class StateMachine {
    public:
        static void init_run(void *obj);
        static void idle_entry(void *obj);
        static void idle_run(void *obj);
        static void test_entry(void *obj);
        static void test_run(void *obj);
        static void test_exit(void *obj);
        static void complete_entry(void *obj);
        static void complete_run(void *obj);
        static void complete_exit(void *obj);
        static void cancel_entry(void *obj);
        static void cancel_run(void *obj);
        static void cancel_exit(void *obj);
};

void state_machine_init();