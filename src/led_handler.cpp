#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/pwm.h>
#include <led_handler.h>

LOG_MODULE_REGISTER(led_handler, LOG_LEVEL_INF);

#define LED1 DT_ALIAS(led0) // LED indicating powered on, error has occured, or low battery
#define LED2 DT_ALIAS(led0) // LED indicating state of device

#define STEP_SIZE PWM_USEC(2000)

static const struct gpio_dt_spec power_led = GPIO_DT_SPEC_GET(LED1, gpios);
static const struct gpio_dt_spec state_led = GPIO_DT_SPEC_GET(LED2, gpios);

static const struct pwm_dt_spec red_pwm_led =
	PWM_DT_SPEC_GET(DT_ALIAS(red_pwm_led));
static const struct pwm_dt_spec green_pwm_led =
	PWM_DT_SPEC_GET(DT_ALIAS(green_pwm_led));
static const struct pwm_dt_spec blue_pwm_led =
	PWM_DT_SPEC_GET(DT_ALIAS(blue_pwm_led));
static const struct pwm_dt_spec yellow_pwm_led =
	PWM_DT_SPEC_GET(DT_ALIAS(yellow_pwm_led));
static const struct pwm_dt_spec white_pwm_led =
	PWM_DT_SPEC_GET(DT_ALIAS(white_pwm_led));

// IDLE state LED
void set_led_blue() {
    uint8_t err;

    err = pwm_set_pulse_dt(&blue_pwm_led, STEP_SIZE);
    if (!err) {
        printk("Error %d: blue write failed\n", err);
        return;
    }
}

// TEST state LED
void set_led_yellow() {
    uint8_t err;

    err = pwm_set_pulse_dt(&yellow_pwm_led, STEP_SIZE);
    if (!err) {
        printk("Error %d: yellow write failed\n", err);
        return;
    }
}

// PROCESS state LED
void set_led_flash_green() {
    uint8_t err;
    uint32_t flash_green;

    for (flash_green = 0U; flash_green <= green_pwm_led.period; flash_green += STEP_SIZE) {
        err = pwm_set_pulse_dt(&green_pwm_led, flash_green);
        if (!err) {
            printk("Error %d: flash green write failed\n", err);
            return;
        }
    }
}

// COMPLETE state LED
void set_led_solid_green() {
    uint8_t err;

    err = pwm_set_pulse_dt(&green_pwm_led, STEP_SIZE);
    if (!err) {
        printk("Error %d: solid green write failed\n", err);
        return;
    }
}

// CANCEL state LED
void set_led_flash_red() {
    uint8_t err;
    uint32_t flash_red;

    for (flash_red = 0U; flash_red <= red_pwm_led.period; flash_red += STEP_SIZE) {
        err = pwm_set_pulse_dt(&red_pwm_led, flash_red);
        if (!err) {
            printk("Error %d: flash red write failed\n", err);
            return;
        }
    }
}

// ERROR INDICATION LED
void set_led_solid_red() {
    uint8_t err;

    err = pwm_set_pulse_dt(&red_pwm_led, STEP_SIZE);
    if (!err) {
        printk("Error %d: solid red write failed\n", err);
        return;
    }
}

// POWER ON LED
void set_led_white() {
    uint8_t err;

    err = pwm_set_pulse_dt(&white_pwm_led, STEP_SIZE);
    if (!err) {
        printk("Error %d: white write failed\n", err);
        return;
    }
}

// LOW BATTERY INDICATION LED
void set_led_blue_white(bool low_battery) {
    uint8_t err;
    uint32_t white;
    uint32_t blue;

    while (low_battery) {
        err = pwm_set_pulse_dt(&white_pwm_led, STEP_SIZE);
        if (err != 0) {
            printk("Error %d: white write failed\n", err);
            return;
        }

        k_sleep(K_MSEC(1000));

        err = pwm_set_pulse_dt(&blue_pwm_led, STEP_SIZE);
        if (err != 0) {
            printk("Error %d: blue write failed\n",
                    err);
            return;
        }
    }
}

void led_init() {
    uint8_t err;

    // Check that led's are ready
    if (!gpio_is_ready_dt(&power_led) || !gpio_is_ready_dt(&state_led)) {
        LOG_ERR("Error: button LED %s is not ready", state_led.port->name);
		return;
	}

    // Configure LED's
    err = gpio_pin_configure_dt(&power_led, GPIO_OUTPUT_ACTIVE);
	if (err < 0) {
		LOG_ERR("Error %d: failed to configure %s pin %d", err, power_led.port->name, power_led.pin);
        return;
	}

    err = gpio_pin_configure_dt(&state_led, GPIO_OUTPUT_ACTIVE);
	if (err < 0) {
		LOG_ERR("Error %d: failed to configure %s pin %d", err, state_led.port->name, state_led.pin);
        return;
	}

    return;
}