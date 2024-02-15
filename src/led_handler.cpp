#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/pwm.h>
#include <led_handler.h>

LOG_MODULE_REGISTER(led_handler, LOG_LEVEL_DBG);

#define STEP_SIZE PWM_USEC(2000)

static const struct pwm_dt_spec red_pwm_led =
	PWM_DT_SPEC_GET(DT_ALIAS(redled));
static const struct pwm_dt_spec green_pwm_led =
	PWM_DT_SPEC_GET(DT_ALIAS(greenled));
static const struct pwm_dt_spec blue_pwm_led =
	PWM_DT_SPEC_GET(DT_ALIAS(blueled));


// IDLE state LED
void set_led_blue() {
    int err;

    LOG_DBG("set LED to blue");

    // Turn on blue LED
    err = pwm_set_pulse_dt(&blue_pwm_led, STEP_SIZE);
    if (err) {
        printk("Error %d: blue write failed\n", err);
        return;
    }

    // Turn off green and red LED's
    err = pwm_set_pulse_dt(&red_pwm_led, 0);
    if (err) {
        printk("Error %d: blue write failed\n", err);
        return;
    }

    err = pwm_set_pulse_dt(&green_pwm_led, 0);
    if (err) {
        printk("Error %d: blue write failed\n", err);
        return;
    }
}

// TEST state LED
void set_led_yellow() {
    int err;

    LOG_DBG("set LED to yellow");

    // Turn on red and green LED's for yellow
    err = pwm_set_pulse_dt(&green_pwm_led, STEP_SIZE);
    if (err) {
        printk("Error %d: solid green write failed\n", err);
        return;
    }

    err = pwm_set_pulse_dt(&red_pwm_led, STEP_SIZE+(STEP_SIZE*1.5));
    if (err) {
        printk("Error %d: solid red write failed\n", err);
        return;
    }
    
    // Turn off blue LED
    err = pwm_set_pulse_dt(&blue_pwm_led, 0);
    if (err) {
        printk("Error %d: blue write failed\n", err);
        return;
    }
}

// PROCESS state LED
void set_led_flash_green() {
    int err;
    uint32_t flash_green;

    LOG_DBG("set LED to flashing green");

    // Turn off red and blue LED's
    err = pwm_set_pulse_dt(&red_pwm_led, 0);
    if (err) {
        printk("Error %d: solid red write failed\n", err);
        return;
    }
    
    err = pwm_set_pulse_dt(&blue_pwm_led, 0);
    if (err) {
        printk("Error %d: blue write failed\n", err);
        return;
    }

    // Turn on green LED flashing
   for (flash_green = 0U; flash_green <= green_pwm_led.period; flash_green += STEP_SIZE) {
        // Change loop to run forever in LED thread until state is changed // set interrupt
        err = pwm_set_pulse_dt(&green_pwm_led, STEP_SIZE);
        if (err) {
            printk("Error %d: flash green write failed\n", err);
            return;
        }

        k_sleep(K_MSEC(1000));

        err = pwm_set_pulse_dt(&green_pwm_led, 0);
        if (err) {
            printk("Error %d: flash green write failed\n", err);
            return;
        }
    }
}

// COMPLETE state LED
void set_led_solid_green() {
    int err;

    LOG_DBG("set LED to solid green");

    // Turn on green LED
    err = pwm_set_pulse_dt(&green_pwm_led, STEP_SIZE);
    if (err) {
        printk("Error %d: solid green write failed\n", err);
        return;
    }

    // Turn off red and blue LED's
    err = pwm_set_pulse_dt(&red_pwm_led, 0);
    if (err) {
        printk("Error %d: solid red write failed\n", err);
        return;
    }
    
    err = pwm_set_pulse_dt(&blue_pwm_led, 0);
    if (err) {
        printk("Error %d: blue write failed\n", err);
        return;
    }
}

// CANCEL state LED
void set_led_flash_red() {
    int err;
    uint32_t flash_red;

    LOG_DBG("set LED to flashing red");

    // Turn on red flashing LED
    for (flash_red = 0U; flash_red <= red_pwm_led.period; flash_red += STEP_SIZE) {
        err = pwm_set_pulse_dt(&red_pwm_led, flash_red);
        if (err) {
            printk("Error %d: flash red write failed\n", err);
            return;
        }
    }

    // Turn off green and blue LED's
    err = pwm_set_pulse_dt(&green_pwm_led, 0);
    if (err) {
        printk("Error %d: solid green write failed\n", err);
        return;
    }
    
    err = pwm_set_pulse_dt(&blue_pwm_led, 0);
    if (err) {
        printk("Error %d: blue write failed\n", err);
        return;
    }
}

// ERROR INDICATION LED
void set_led_solid_red() {
    int err;

    LOG_DBG("set LED to solid red");

    // Turn on red LED
    err = pwm_set_pulse_dt(&red_pwm_led, STEP_SIZE);
    if (err) {
        printk("Error %d: solid red write failed\n", err);
        return;
    }

    // Turn off green and blue LED's
    err = pwm_set_pulse_dt(&green_pwm_led, 0);
    if (err) {
        printk("Error %d: solid green write failed\n", err);
        return;
    }
    
    err = pwm_set_pulse_dt(&blue_pwm_led, 0);
    if (err) {
        printk("Error %d: blue write failed\n", err);
        return;
    }
}

// POWER ON LED
void set_led_white() {
    int err;

    LOG_DBG("set LED to white");

    // Turn on blue, red, and green for white
    err = pwm_set_pulse_dt(&blue_pwm_led, STEP_SIZE);
        if (err != 0) {
            printk("Error %d: blue write failed\n",
                    err);
            return;
        }

    err = pwm_set_pulse_dt(&green_pwm_led, STEP_SIZE);
    if (err) {
        printk("Error %d: solid green write failed\n", err);
        return;
    }

    err = pwm_set_pulse_dt(&red_pwm_led, STEP_SIZE);
    if (err) {
        printk("Error %d: solid red write failed\n", err);
        return;
    }
}

// LOW BATTERY INDICATION LED
void set_led_blue_white(bool low_battery) {
    int err;

    LOG_DBG("set LED to blue and white alternating");

    while (low_battery) {

        // Turn on blue, red, and green for white
        err = pwm_set_pulse_dt(&blue_pwm_led, STEP_SIZE);
            if (err != 0) {
                printk("Error %d: blue write failed\n",
                        err);
                return;
            }

        err = pwm_set_pulse_dt(&green_pwm_led, STEP_SIZE);
        if (err) {
            printk("Error %d: solid green write failed\n", err);
            return;
        }

        err = pwm_set_pulse_dt(&red_pwm_led, STEP_SIZE);
        if (err) {
            printk("Error %d: solid red write failed\n", err);
            return;
        }

        k_sleep(K_MSEC(10000));

        // Turn off red and green for blue only
        err = pwm_set_pulse_dt(&green_pwm_led, 0);
        if (err) {
            printk("Error %d: solid green write failed\n", err);
            return;
        }

        err = pwm_set_pulse_dt(&red_pwm_led, 0);
        if (err) {
            printk("Error %d: solid red write failed\n", err);
            return;
        }
    }
}

void led_init() {
    // Check that led's are ready
    if (!device_is_ready(red_pwm_led.dev) || !device_is_ready(green_pwm_led.dev) || !device_is_ready(blue_pwm_led.dev)) {
        LOG_ERR("Error: LED is not ready");
		return;
	}

    return;
}