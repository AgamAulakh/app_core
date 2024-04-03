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

#define BUTTON1 DT_ALIAS(button1) // Button for initiating or cancelling test P1.00
#if !DT_NODE_HAS_STATUS(BUTTON1, okay)
#error "Unsupported board: DATA_ENABLE devicetree alias is not defined"
#endif

#define BUTTON2 DT_ALIAS(button2) // Button for initiating or cancelling test P1.00
#if !DT_NODE_HAS_STATUS(BUTTON2, okay)
#error "Unsupported board: DATA_ENABLE devicetree alias is not defined"
#endif

/* List of events */
#define EVENT_BTN1_PRESS BIT(0)
#define EVENT_BTN2_PRESS BIT(1)
#define EVENT_SIG_PROC_COMPLETE BIT(2)
#define EVENT_RETURN_TO_IDLE BIT(3)

namespace Button1 {
    static const struct gpio_dt_spec button1 = GPIO_DT_SPEC_GET_OR(BUTTON1, gpios, {0});
    static struct gpio_callback button_cb_data;

    void init();
    void button_press(const struct device *dev, struct gpio_callback *cb, uint32_t pins);
};

namespace Button2 {
    static const struct gpio_dt_spec button2 = GPIO_DT_SPEC_GET_OR(BUTTON2, gpios, {0});
    static struct gpio_callback button_cb_data;

    void init();
    void button_press(const struct device *dev, struct gpio_callback *cb, uint32_t pins);
};

class StateMachine {
    public:
        static void idle_entry(void *obj);
        static void idle_run(void *obj);
        static void idle_exit(void *obj);
        static void test_entry(void *obj);
        static void test_run(void *obj);
        static void test_exit(void *obj);
        static void complete_entry(void *obj);
        static void complete_run(void *obj);
        static void complete_exit(void *obj);
        static void cancel_entry(void *obj);
        static void cancel_run(void *obj);
        static void cancel_exit(void *obj);
        static void demo_entry(void *obj);
        static void demo_run(void *obj);
        static void demo_exit(void *obj);
};

void state_machine_init();