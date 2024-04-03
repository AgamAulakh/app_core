#pragma once
// #include "display_st7735r.h"
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
#include <lvgl.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include "core/Thread.h"

#define HID_THREAD_STACK_SIZE_B 4096
#define HID_THREAD_PRIORITY 1
#define HID_THREAD_MSG_Q_DEPTH 10


class HIDThread : public Thread<HID_THREAD_MSG_Q_DEPTH>  {
private:
    HIDThread();
    ~HIDThread() = default;

    HIDThread(const HIDThread &) = delete;
    HIDThread& operator=(const HIDThread) = delete;

public:
    static HIDThread& GetInstance() {
        static HIDThread instance;
        return instance;
    };

    enum HDIThreadMessage : uint8_t {
        DISPLAY_STARTUP,
        DISPLAY_TESTING,
        DISPLAY_RESULTS,
        DISPLAY_CANCEL,
    };

    void Initialize() override;
    void Run() override;
};