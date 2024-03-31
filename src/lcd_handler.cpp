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
#include <zephyr/drivers/display.h>
#include <sstream>

using namespace std;

LOG_MODULE_REGISTER(lcd_handler, LOG_LEVEL_DBG);

const struct device *display_dev;
lv_obj_t *hello_world_label;
lv_obj_t *testing_label;

static uint8_t current_dev_state = 0;

static uint8_t buf[1024];
static const struct display_buffer_descriptor buf_desc = {
	.buf_size = sizeof(buf),
	.width    = 5,
	.height   = 5,
	.pitch    = 8,
};

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


enum corner {
	TOP_LEFT,
	TOP_RIGHT,
	BOTTOM_RIGHT,
	BOTTOM_LEFT
};

typedef void (*fill_buffer)(enum corner corner, uint8_t grey, uint8_t *buf,
			    size_t buf_size);


#ifdef CONFIG_ARCH_POSIX
static void posix_exit_main(int exit_code)
{
#if CONFIG_TEST
	if (exit_code == 0) {
		LOG_INF("PROJECT EXECUTION SUCCESSFUL");
	} else {
		LOG_INF("PROJECT EXECUTION FAILED");
	}
#endif
	posix_exit(exit_code);
}
#endif

static void fill_buffer_argb8888(enum corner corner, uint8_t grey, uint8_t *buf,
				 size_t buf_size)
{
	uint32_t color = 0;

	switch (corner) {
	case TOP_LEFT:
		color = 0x00FF0000u;
		break;
	case TOP_RIGHT:
		color = 0x0000FF00u;
		break;
	case BOTTOM_RIGHT:
		color = 0x000000FFu;
		break;
	case BOTTOM_LEFT:
		color = grey << 16 | grey << 8 | grey;
		break;
	}

	for (size_t idx = 0; idx < buf_size; idx += 4) {
		*((uint32_t *)(buf + idx)) = color;
	}
}

static void fill_buffer_rgb888(enum corner corner, uint8_t grey, uint8_t *buf,
			       size_t buf_size)
{
	uint32_t color = 0;

	switch (corner) {
	case TOP_LEFT:
		color = 0x00FF0000u;
		break;
	case TOP_RIGHT:
		color = 0x0000FF00u;
		break;
	case BOTTOM_RIGHT:
		color = 0x000000FFu;
		break;
	case BOTTOM_LEFT:
		color = grey << 16 | grey << 8 | grey;
		break;
	}

	for (size_t idx = 0; idx < buf_size; idx += 3) {
		*(buf + idx + 0) = color >> 16;
		*(buf + idx + 1) = color >> 8;
		*(buf + idx + 2) = color >> 0;
	}
}

static uint16_t get_rgb565_color(enum corner corner, uint8_t grey)
{
	uint16_t color = 0;
	uint16_t grey_5bit;

	switch (corner) {
	case TOP_LEFT:
		color = 0xF800u;
		break;
	case TOP_RIGHT:
		color = 0x07E0u;
		break;
	case BOTTOM_RIGHT:
		color = 0x001Fu;
		break;
	case BOTTOM_LEFT:
		grey_5bit = grey & 0x1Fu;
		/* shift the green an extra bit, it has 6 bits */
		color = grey_5bit << 11 | grey_5bit << (5 + 1) | grey_5bit;
		break;
	}
	return color;
}

static void fill_buffer_rgb565(enum corner corner, uint8_t grey, uint8_t *buf,
			       size_t buf_size)
{
	uint16_t color = get_rgb565_color(corner, grey);

	for (size_t idx = 0; idx < buf_size; idx += 2) {
		*(buf + idx + 0) = (color >> 8) & 0xFFu;
		*(buf + idx + 1) = (color >> 0) & 0xFFu;
	}
}

static void fill_buffer_bgr565(enum corner corner, uint8_t grey, uint8_t *buf,
			       size_t buf_size)
{
	uint16_t color = get_rgb565_color(corner, grey);

	for (size_t idx = 0; idx < buf_size; idx += 2) {
		*(uint16_t *)(buf + idx) = color;
	}
}

static void fill_buffer_mono(enum corner corner, uint8_t grey, uint8_t *buf,
			     size_t buf_size)
{
	uint16_t color;

	switch (corner) {
	case BOTTOM_LEFT:
		color = (grey & 0x01u) ? 0xFFu : 0x00u;
		break;
	default:
		color = 0;
		break;
	}

	memset(buf, color, buf_size);
}

void lcd_init(void)
{
	size_t x;
	size_t y;
	size_t rect_w;
	size_t rect_h;
	size_t h_step;
	size_t scale;
	size_t grey_count;
	uint8_t *buf;
	int32_t grey_scale_sleep;
	const struct device *display_dev;
	struct display_capabilities capabilities;
	struct display_buffer_descriptor buf_desc;
	size_t buf_size = 0;
	fill_buffer fill_buffer_fnc = NULL;

     LOG_DBG("LCD init");

	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Device %s not found. Aborting sample.",
			display_dev->name);
#ifdef CONFIG_ARCH_POSIX
		posix_exit_main(1);
#else
		return;
#endif
	}

	LOG_INF("Display sample for %s", display_dev->name);
	display_get_capabilities(display_dev, &capabilities);

	if (capabilities.screen_info & SCREEN_INFO_MONO_VTILED) {
		rect_w = 16;
		rect_h = 8;
	} else {
		rect_w = 2;
		rect_h = 1;
	}

	h_step = rect_h;
	scale = (capabilities.x_resolution / 8) / rect_h;

	rect_w *= scale;
	rect_h *= scale;

	if (capabilities.screen_info & SCREEN_INFO_EPD) {
		grey_scale_sleep = 10000;
	} else {
		grey_scale_sleep = 100;
	}

	buf_size = rect_w * rect_h;

	if (buf_size < (capabilities.x_resolution * h_step)) {
		buf_size = capabilities.x_resolution * h_step;
	}

	switch (capabilities.current_pixel_format) {
	case PIXEL_FORMAT_ARGB_8888:
		fill_buffer_fnc = fill_buffer_argb8888;
		buf_size *= 4;
		break;
	case PIXEL_FORMAT_RGB_888:
		fill_buffer_fnc = fill_buffer_rgb888;
		buf_size *= 3;
		break;
	case PIXEL_FORMAT_RGB_565:
		fill_buffer_fnc = fill_buffer_rgb565;
		buf_size *= 2;
		break;
	case PIXEL_FORMAT_BGR_565:
		fill_buffer_fnc = fill_buffer_bgr565;
		buf_size *= 2;
		break;
	case PIXEL_FORMAT_MONO01:
	case PIXEL_FORMAT_MONO10:
		fill_buffer_fnc = fill_buffer_mono;
		buf_size /= 8;
		break;
	default:
		LOG_ERR("Unsupported pixel format. Aborting sample.");
#ifdef CONFIG_ARCH_POSIX
		posix_exit_main(1);
#else
		return;
#endif
	}

	buf = static_cast<uint8_t *>(k_malloc(buf_size));

	if (buf == NULL) {
		LOG_ERR("Could not allocate memory. Aborting sample.");
#ifdef CONFIG_ARCH_POSIX
		posix_exit_main(1);
#else
		return;
#endif
	}

	(void*)memset(buf, 0xFFu, buf_size);

	buf_desc.buf_size = buf_size;
	buf_desc.pitch = capabilities.x_resolution;
	buf_desc.width = capabilities.x_resolution;
	buf_desc.height = h_step;

	for (int idx = 0; idx < capabilities.y_resolution; idx += h_step) {
		display_write(display_dev, 0, idx, &buf_desc, buf);
	}

	buf_desc.pitch = rect_w;
	buf_desc.width = rect_w;
	buf_desc.height = rect_h;

	fill_buffer_fnc(TOP_LEFT, 0, buf, buf_size);
	x = 0;
	y = 0;
	display_write(display_dev, x, y, &buf_desc, buf);

	fill_buffer_fnc(TOP_RIGHT, 0, buf, buf_size);
	x = capabilities.x_resolution - rect_w;
	y = 0;
	display_write(display_dev, x, y, &buf_desc, buf);

	fill_buffer_fnc(BOTTOM_RIGHT, 0, buf, buf_size);
	x = capabilities.x_resolution - rect_w;
	y = capabilities.y_resolution - rect_h;
	display_write(display_dev, x, y, &buf_desc, buf);

	display_blanking_off(display_dev);

	grey_count = 0;
	x = 0;
	y = capabilities.y_resolution - rect_h;

	while (1) {
		fill_buffer_fnc(BOTTOM_LEFT, grey_count, buf, buf_size);
		display_write(display_dev, x, y, &buf_desc, buf);
		++grey_count;
		k_msleep(grey_scale_sleep);
#if CONFIG_TEST
		if (grey_count >= 1024) {
			break;
		}
#endif
	}

#ifdef CONFIG_ARCH_POSIX
	posix_exit_main(0);
#endif
	return;
}

// void lcd_init(void)
// {
//     LOG_DBG("LCD init");
//     display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
//     if (!device_is_ready(display_dev)) {
// 		LOG_ERR("Device not ready, aborting");
// 		return; 
//     }

//     // display_dev->api.write(display_dev, 0, 0, &buf_desc, &buf);

//     display_write(display_dev, 5, 5, &buf_desc, buf);

//     // st7735r_api.write(display_dev, 0, 0, &buf_desc, &buf);

//     // st7735r_transmit(display_dev, uint8_t cmd, const uint8_t *tx_data, size_t tx_count)

//     // display_blanking_off(display_dev);

//     // hello_world_label = lv_label_create(lv_scr_act());
//     // // testing_label = lv_label_create(lv_scr_act());

//     // // lv_label_set_long_mode(hello_world_label, LV_LABEL_LONG_WRAP);
//     // lv_label_set_text(hello_world_label, "Hello world from EEGALs!");
//     // lv_obj_set_align(hello_world_label, LV_ALIGN_CENTER);
//     // lv_task_handler();
//     // display_blanking_off_api(display_dev);

//     while(1) {
//         // k_msleep(1000);
//         // lv_label_set_text(hello_world_label, "Hello world");
//         // LOG_INF("task handler");
//         // lv_task_handler();
//         // // hello_world_label = lv_label_create(lv_scr_act());
//         // lv_label_set_text(hello_world_label, "Hello world!");
//         // lv_obj_set_align(hello_world_label, LV_ALIGN_CENTER);
//         // // lv_task_handler();
//         // display_blanking_off_api(display_dev);
//         // k_msleep(100);
//     }
// }
