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
#include "Events.h"

using namespace std;

LOG_MODULE_REGISTER(lcd_handler, LOG_LEVEL_DBG);

const struct device *display_dev;
lv_obj_t *display_label;


#define BACKLIGHT_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec backlight_pin = GPIO_DT_SPEC_GET(BACKLIGHT_NODE, gpios);


void LCD::display_init() {
    lv_label_set_text(display_label, "Welcome\nPowering Up");
    lv_obj_set_align(display_label, LV_ALIGN_CENTER);
    lv_task_handler();

    // Allow time for DAQ and Sig Proc threads to initialize
    k_msleep(5000);

    lv_label_set_text(display_label, "Please press the\nbutton to begin");
    lv_obj_set_align(display_label, LV_ALIGN_CENTER);
    lv_task_handler();
}

void LCD::display_testing() {
    lv_label_set_text(display_label, "Testing is starting,\nplease remain still");
    lv_obj_set_align(display_label, LV_ALIGN_CENTER);
    lv_task_handler();

    k_msleep(2000);

    lv_label_set_text(display_label, "Testing in progress,\n DO NOT move!");
    lv_obj_set_align(display_label, LV_ALIGN_CENTER);
    lv_task_handler();
}

void LCD::display_complete(float *alphaPower, float *betaPower, float *deltaPower, float *thetaPower) {
    lv_label_set_text(display_label, "Testing complete");
    lv_obj_set_align(display_label, LV_ALIGN_CENTER);
    lv_task_handler();

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

    k_sleep(K_MSEC(3000));

    // std::string formattedResults = std::string("%c\n%c\n%c\n%c", alphaResult.c_str(), betaResult.c_str(), deltaResult.c_str(), thetaResult.c_str());

    std::string combined = alphaResult + std::string("\n") + betaResult + std::string("\n") + deltaResult + std::string("\n") + thetaResult;

    lv_label_set_text(display_label, combined.c_str());
    lv_obj_set_align(display_label, LV_ALIGN_CENTER);
    lv_task_handler();

    k_sleep(K_MSEC(3000));

    lv_label_set_text(display_label, combined.c_str());
    lv_obj_set_align(display_label, LV_ALIGN_CENTER);
    lv_task_handler();
    
    k_sleep(K_MSEC(3000));

    lv_label_set_text(display_label, combined.c_str());
    lv_obj_set_align(display_label, LV_ALIGN_CENTER);
    lv_task_handler();

    k_sleep(K_MSEC(3000));

    lv_label_set_text(display_label, combined.c_str());
    lv_obj_set_align(display_label, LV_ALIGN_CENTER);
    lv_task_handler();

    k_sleep(K_MSEC(3000));

    return_to_idle();
}

void LCD::display_cancel() {
    lv_label_set_text(display_label, "Test cancelled,\nreturning to home");
    lv_obj_set_align(display_label, LV_ALIGN_CENTER);
    lv_task_handler();
}

void LCD::display_demo_mode() {
    lv_label_set_text(display_label, "Demo Mode");
    lv_obj_set_align(display_label, LV_ALIGN_CENTER);
    lv_task_handler();
}


void lcd_init(void)
{
    int ret;

    LOG_DBG("LCD init");
    display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(display_dev)) {
		LOG_ERR("Device not ready, aborting");
		return; 
    }

	if (!gpio_is_ready_dt(&backlight_pin)) {
		return;
	}

	ret = gpio_pin_configure_dt(&backlight_pin, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return;
	}

    display_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(display_label, &lv_font_montserrat_16, 0);

    display_blanking_off(display_dev);

	return;
}
