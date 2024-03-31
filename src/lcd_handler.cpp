#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <lcd_handler.h>
#include <sstream>

using namespace std;

LOG_MODULE_REGISTER(lcd_handler, LOG_LEVEL_DBG);

const struct device *display_dev;
lv_obj_t *hello_world_label;
lv_obj_t *testing_label;

static uint8_t current_dev_state = 0;

void LCD::display_init() {
    lv_label_set_text(testing_label, "Welcome: EEG Data Collection Device");
    lv_label_set_long_mode(testing_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_align(testing_label, LV_ALIGN_CENTER);
    lv_task_handler();
    display_blanking_off(display_dev);

    k_sleep(K_MSEC(1000));

    lv_label_set_text(testing_label, "Please press the button to begin");
    lv_label_set_long_mode(testing_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_align(testing_label, LV_ALIGN_CENTER);
    lv_task_handler();
    display_blanking_off(display_dev);
}

void LCD::display_testing() {
    lv_label_set_text(testing_label, "Data collection will begin shortly, please remain still. If at any time during data collection you wish to cancel the test, please press the button again");
    lv_label_set_long_mode(testing_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_align(testing_label, LV_ALIGN_CENTER);
    lv_task_handler();
    display_blanking_off(display_dev);

    k_sleep(K_MSEC(1000));

    lv_label_set_text(testing_label, "Data collection in progress, please DO NOT move!");
    lv_label_set_long_mode(testing_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_align(testing_label, LV_ALIGN_CENTER);
    lv_task_handler();
    display_blanking_off(display_dev);
}

void LCD::display_processing() {
    lv_label_set_text(testing_label, "Data collection is complete, you are free to move as the data is processed. Please wait for results");
    lv_label_set_long_mode(testing_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_align(testing_label, LV_ALIGN_CENTER);
    lv_task_handler();
    display_blanking_off(display_dev);
}

void LCD::display_complete(float *alphaPower, float *betaPower, float *deltaPower, float *thetaPower) {
    lv_label_set_text(testing_label, "Data processing is complete, your results will now be displayed");
    lv_label_set_long_mode(testing_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_align(testing_label, LV_ALIGN_CENTER);
    lv_task_handler();
    display_blanking_off(display_dev);

    std::ostringstream myString1;
    myString1 << "Alpha: " << alphaPower;
    std::string alphaResult = myString1.str();

    std::ostringstream myString2;
    myString2 << "Beta: " << betaPower;
    std::string betaResult = myString2.str();

    std::ostringstream myString3;
    myString3 << "Delta: " << deltaPower;
    std::string deltaResult = myString3.str();

    std::ostringstream myString4;
    myString4 << "Theta: " << thetaPower;
    std::string thetaResult = myString4.str();

    k_sleep(K_MSEC(1000));

    lv_label_set_text(testing_label, alphaResult.c_str());
    lv_label_set_long_mode(testing_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_align(testing_label, LV_ALIGN_CENTER);
    lv_task_handler();
    display_blanking_off(display_dev);

    k_sleep(K_MSEC(1000));

    lv_label_set_text(testing_label, betaResult.c_str());
    lv_label_set_long_mode(testing_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_align(testing_label, LV_ALIGN_CENTER);
    lv_task_handler();
    display_blanking_off(display_dev);
    
    k_sleep(K_MSEC(1000));

    lv_label_set_text(testing_label, deltaResult.c_str());
    lv_label_set_long_mode(testing_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_align(testing_label, LV_ALIGN_CENTER);
    lv_task_handler();
    display_blanking_off(display_dev);

    k_sleep(K_MSEC(1000));

    lv_label_set_text(testing_label, thetaResult.c_str());
    lv_label_set_long_mode(testing_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_align(testing_label, LV_ALIGN_CENTER);
    lv_task_handler();
    display_blanking_off(display_dev);
}

void LCD::display_cancel() {
    lv_label_set_text(testing_label, "Test cancelled, returning to home");
    lv_label_set_long_mode(testing_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_align(testing_label, LV_ALIGN_CENTER);
    lv_task_handler();
    display_blanking_off(display_dev);

    k_sleep(K_MSEC(1000));

    lv_label_set_text(testing_label, "results");
    lv_label_set_long_mode(testing_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_align(testing_label, LV_ALIGN_CENTER);
    lv_task_handler();
    display_blanking_off(display_dev);
}

void lcd_init(void)
{
    LOG_DBG("LCD init");
    display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(display_dev)) {
		LOG_ERR("Device not ready, aborting");
		return; 
    }

    // display_blanking_off(display_dev);

    hello_world_label = lv_label_create(lv_scr_act());
    // testing_label = lv_label_create(lv_scr_act());

    // lv_label_set_long_mode(hello_world_label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(hello_world_label, "Hello world from EEGALs!");
    lv_obj_set_align(hello_world_label, LV_ALIGN_CENTER);
    lv_task_handler();
    display_blanking_off_api(display_dev);

    while(1) {
        k_msleep(1000);
        lv_label_set_text(hello_world_label, "Hello world");
        LOG_INF("task handler");
        lv_task_handler();
        // // hello_world_label = lv_label_create(lv_scr_act());
        // lv_label_set_text(hello_world_label, "Hello world!");
        // lv_obj_set_align(hello_world_label, LV_ALIGN_CENTER);
        // // lv_task_handler();
        // display_blanking_off_api(display_dev);
        // k_msleep(100);
    }
}
