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

LOG_MODULE_REGISTER(lcd_handler, LOG_LEVEL_DBG);

const struct device *display_dev;
lv_obj_t *hello_world_label;
lv_obj_t *testing_label;

static uint8_t current_dev_state = 0;

void LCD::display_testing() {
    lv_label_set_text(testing_label, "Data collection in progress, please DO NOT move!");
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
    testing_label = lv_label_create(lv_scr_act());

    // lv_label_set_long_mode(hello_world_label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(hello_world_label, "Hello world from EEGALs!");
    lv_obj_set_align(hello_world_label, LV_ALIGN_CENTER);
    lv_task_handler();
    display_blanking_on(display_dev);

    while(1) {
        lv_task_handler();
        k_msleep(5);
    }
}

