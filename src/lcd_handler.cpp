#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <lvgl.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <lcd_handler.h>
#include "Events.h"

LOG_MODULE_REGISTER(lcd_handler, LOG_LEVEL_DBG);

// static fields
Result LCD::most_recent_result = {0};
char LCD::result_str[100] = {0};
// MessageQueue<Result, LCD_RESULTS_MSG_Q_DEPTH> LCD::result_queue;

// device tree fields
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
}

void LCD::display_idle() {
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

void LCD::update_most_recent_result(Result& to_update) {
    for (int i = 0; i < num_electrodes; i++) {
        most_recent_result.band_powers[i].theta = to_update.band_powers[i].theta;
        most_recent_result.band_powers[i].delta = to_update.band_powers[i].delta;
        most_recent_result.band_powers[i].alpha = to_update.band_powers[i].alpha;
        most_recent_result.band_powers[i].beta = to_update.band_powers[i].beta;
        LOG_INF("theta: %f delta: %f alpha: %f beta: %f", to_update.band_powers[i].theta, to_update.band_powers[i].delta, to_update.band_powers[i].alpha, to_update.band_powers[i].beta);
    }
    LOG_INF("is abnormal: %d", to_update.is_abnormal);
    most_recent_result.is_abnormal = to_update.is_abnormal;
}
void LCD::display_complete() {
    lv_label_set_text(display_label, "Testing complete");
    lv_obj_set_align(display_label, LV_ALIGN_CENTER);
    lv_task_handler();
    k_sleep(K_MSEC(1500));

    // try to update current result with data in queue
    // if get from queue fails, the most_recent_result should be the same as previous (is not consumed)
    // if get from queue passes, then we have a new result!
    // if(result_queue.get_with_blocking_wait(most_recent_result)) {
    //     most_recent_result = static_cast<Result>(most_recent_result);
    //     LOG_INF("updated the result");
    // }
    // else {
    //     LOG_ERR("did not read the result from queue");
    // }

    lv_obj_set_style_text_font(display_label, &lv_font_montserrat_14, 0);
    for (int electrode = 0; electrode < num_electrodes; electrode++) {
        sprintf(result_str, "Electrode: %d\nDelta: %d.%d\nTheta: %d.%d\nAlpha: %d.%d\nBeta: %d.%d",
            (electrode + 1),
            Utils::get_whole_number_from_float(most_recent_result.band_powers[electrode].delta),
            Utils::get_decimal_number_from_float(most_recent_result.band_powers[electrode].delta, 3),
            Utils::get_whole_number_from_float(most_recent_result.band_powers[electrode].theta),
            Utils::get_decimal_number_from_float(most_recent_result.band_powers[electrode].theta, 3),
            Utils::get_whole_number_from_float(most_recent_result.band_powers[electrode].alpha),
            Utils::get_decimal_number_from_float(most_recent_result.band_powers[electrode].alpha, 3),
            Utils::get_whole_number_from_float(most_recent_result.band_powers[electrode].beta),
            Utils::get_decimal_number_from_float(most_recent_result.band_powers[electrode].beta, 3)
        );

        // LOG_INF("DUMP: %.3f, %.3f, %.3f, %.3f", most_recent_result.band_powers[electrode].delta, most_recent_result.band_powers[electrode].theta, most_recent_result.band_powers[electrode].alpha, most_recent_result.band_powers[electrode].beta);
        lv_label_set_text(display_label, result_str);
        lv_obj_set_align(display_label, LV_ALIGN_CENTER);
        lv_task_handler();
        k_sleep(K_MSEC(1500));
    }

    if(most_recent_result.is_abnormal) {
        lv_label_set_text(display_label, "Results Indicate\nABNORMAL\nBrain State");
    }
    else {
        lv_label_set_text(display_label, "Results Indicate\nNormal\nBrain State");
    }
    lv_obj_set_align(display_label, LV_ALIGN_CENTER);
    lv_task_handler();

    k_sleep(K_MSEC(4000));

    lv_obj_set_style_text_font(display_label, &lv_font_montserrat_16, 0);

    return_to_idle();
}

void LCD::display_cancel() {
    lv_label_set_text(display_label, "Test cancelled,\nreturning to home");
    lv_obj_set_align(display_label, LV_ALIGN_CENTER);
    lv_task_handler();
}

void LCD::prepare_queue_for_new_result() {
    // result_queue.resetMessageQueue();
    LOG_ERR("CCRAAAZZYYYYY YOU'RE DELETING THIS QUEUE??????????????????????");
}

void LCD::display_demo_mode() {
    lv_label_set_text(display_label, "Demo Mode");
    lv_obj_set_align(display_label, LV_ALIGN_CENTER);
    lv_task_handler();
}


void LCD::lcd_init(void)
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
