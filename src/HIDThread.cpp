#include "HIDThread.h"
#include "lcd_handler.h"

LOG_MODULE_REGISTER(HIDThread, LOG_LEVEL_INF);
K_THREAD_STACK_DEFINE(HID_stack_area, HID_THREAD_STACK_SIZE_B);

struct k_thread HID_thread_data;


HIDThread::HIDThread() {};


void HIDThread::Initialize() {
    LOG_DBG("HID::%s -- initializing HID", __FUNCTION__);

    if (id == nullptr) {
        LOG_DBG("HID::%s -- making thread", __FUNCTION__);
        id = k_thread_create(
            &HID_thread_data, HID_stack_area,
            K_THREAD_STACK_SIZEOF(HID_stack_area),
            HIDThread::RunThreadSequence,
            this, NULL, NULL,
            HID_THREAD_PRIORITY, 0, K_NO_WAIT
        );

        k_thread_name_set(id, "HIDThread");
        LOG_DBG("HID::%s -- thread create successful", __FUNCTION__);
    }
};

void HIDThread::Run() {
    uint8_t message = 0;

    LCD::lcd_init();

    while (true) {
        if (message_queue.get_with_blocking_wait(message)) {
            uint8_t message_enum = static_cast<HDIThreadMessage>(message);

		    LOG_DBG("HDI::%s -- received message: %u at: %u ms", __FUNCTION__, message_enum, k_uptime_get_32());

            switch (message_enum) {
                case DISPLAY_STARTUP:
                    LCD::display_init();
                    break;
                case DISPLAY_TESTING:
                    LCD::display_testing();
                    break;
                case DISPLAY_RESULTS:
                    LCD::display_complete();
                    break;
                case DISPLAY_CANCEL:
                    LCD::display_cancel();
                    break;
                case DISPLAY_DEMO:
                    LCD::display_demo_mode();
                    break;
                default:
                    break;
            }
        }
    }
};
