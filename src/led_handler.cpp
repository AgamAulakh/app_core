#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/pwm.h>
#include <led_handler.h>

LOG_MODULE_REGISTER(led_handler, LOG_LEVEL_DBG);

#define STEP_SIZE PWM_USEC(2000)

// IDLE state LED
void LED1::set_blue() {
    int err;

    LOG_DBG("set LED to blue");

    // Turn on blue LED
    err = pwm_set_pulse_dt(&LED1::blue_pwm_led, STEP_SIZE);
    if (err) {
        printk("Error %d: blue write failed\n", err);
        return;
    }

    // Turn off green and red LED's
    err = pwm_set_pulse_dt(&LED1::red_pwm_led, 0);
    if (err) {
        printk("Error %d: blue write failed\n", err);
        return;
    }

    err = pwm_set_pulse_dt(&LED1::green_pwm_led, 0);
    if (err) {
        printk("Error %d: blue write failed\n", err);
        return;
    }
}

// TEST state LED
void LED1::set_yellow() {
    int err;

    LOG_DBG("set LED to yellow");

    // Turn on red and green LED's for yellow
    err = pwm_set_pulse_dt(&LED1::green_pwm_led, STEP_SIZE);
    if (err) {
        printk("Error %d: solid green write failed\n", err);
        return;
    }

    err = pwm_set_pulse_dt(&LED1::red_pwm_led, STEP_SIZE+(STEP_SIZE*1.5));
    if (err) {
        printk("Error %d: solid red write failed\n", err);
        return;
    }
    
    // Turn off blue LED
    err = pwm_set_pulse_dt(&LED1::blue_pwm_led, 0);
    if (err) {
        printk("Error %d: blue write failed\n", err);
        return;
    }
}

// COMPLETE state LED
void LED1::set_solid_green() {
    int err;

    LOG_DBG("set LED to solid green");

    // Turn on green LED
    err = pwm_set_pulse_dt(&LED1::green_pwm_led, STEP_SIZE);
    if (err) {
        printk("Error %d: solid green write failed\n", err);
        return;
    }

    // Turn off red and blue LED's
    err = pwm_set_pulse_dt(&LED1::red_pwm_led, 0);
    if (err) {
        printk("Error %d: solid red write failed\n", err);
        return;
    }
    
    err = pwm_set_pulse_dt(&LED1::blue_pwm_led, 0);
    if (err) {
        printk("Error %d: blue write failed\n", err);
        return;
    }

    k_sleep(K_MSEC(1000));
}

// CANCEL state LED
void LED1::set_flash_red() {
    int err;
    uint32_t flash_red;

    LOG_DBG("set LED to flashing red");

    // Turn on red flashing LED
    for (flash_red = 0U; flash_red <= LED1::red_pwm_led.period; flash_red += STEP_SIZE) {
        err = pwm_set_pulse_dt(&LED1::red_pwm_led, flash_red);
        if (err) {
            printk("Error %d: flash red write failed\n", err);
            return;
        }
    }

    // Turn off green and blue LED's
    err = pwm_set_pulse_dt(&LED1::green_pwm_led, 0);
    if (err) {
        printk("Error %d: solid green write failed\n", err);
        return;
    }
    
    err = pwm_set_pulse_dt(&LED1::blue_pwm_led, 0);
    if (err) {
        printk("Error %d: blue write failed\n", err);
        return;
    }
}

// ERROR INDICATION LED
void LED1::set_solid_red() {
    int err;

    LOG_DBG("set LED to solid red");

    // Turn on red LED
    err = pwm_set_pulse_dt(&LED1::red_pwm_led, STEP_SIZE);
    if (err) {
        printk("Error %d: solid red write failed\n", err);
        return;
    }

    // Turn off green and blue LED's
    err = pwm_set_pulse_dt(&LED1::green_pwm_led, 0);
    if (err) {
        printk("Error %d: solid green write failed\n", err);
        return;
    }
    
    err = pwm_set_pulse_dt(&LED1::blue_pwm_led, 0);
    if (err) {
        printk("Error %d: blue write failed\n", err);
        return;
    }
}

// DEMO MODE LED
void LED1::set_solid_purple() {
    int err;

    LOG_DBG("set LED to purple");

    // Turn on blue and red LEDs for purple
    err = pwm_set_pulse_dt(&LED1::blue_pwm_led, STEP_SIZE);
        if (err != 0) {
            printk("Error %d: blue write failed\n",
                    err);
            return;
        }

    err = pwm_set_pulse_dt(&LED1::red_pwm_led, STEP_SIZE);
    if (err) {
        printk("Error %d: solid red write failed\n", err);
        return;
    }

    // Turn off green LED
    err = pwm_set_pulse_dt(&LED1::green_pwm_led, 0);
    if (err) {
        printk("Error %d: solid green write failed\n", err);
        return;
    }
}

// DEMO MODE TESTING LED
void LED1::set_flash_purple() {
    int err;
    uint32_t flash_purple;

    LOG_DBG("set LED to purple flashing");

    // Turn off green LED
    err = pwm_set_pulse_dt(&LED1::green_pwm_led, 0);
    if (err) {
        printk("Error %d: solid green write failed\n", err);
        return;
    }

    for (flash_purple = 0U; flash_purple <= LED1::red_pwm_led.period; flash_purple += STEP_SIZE) {
        // Turn on blue and red LEDs for purple
        err = pwm_set_pulse_dt(&LED1::blue_pwm_led, flash_purple);
        if (err != 0) {
            printk("Error %d: blue write failed\n", err);
            return;
        }

        err = pwm_set_pulse_dt(&LED1::red_pwm_led, flash_purple);
        if (err) {
            printk("Error %d: solid red write failed\n", err);
            return;
        }

        k_msleep(500);

        err = pwm_set_pulse_dt(&LED1::blue_pwm_led, 0);
        if (err != 0) {
            printk("Error %d: blue write failed\n", err);
            return;
        }

        err = pwm_set_pulse_dt(&LED1::red_pwm_led, 0);
        if (err) {
            printk("Error %d: solid red write failed\n", err);
            return;
        }
    }
}

// POWER ON LED
void LED1::set_white() {
    int err;

    LOG_DBG("set LED to white");

    // Turn on blue, red, and green for white
    err = pwm_set_pulse_dt(&LED1::blue_pwm_led, STEP_SIZE);
        if (err != 0) {
            printk("Error %d: blue write failed\n",
                    err);
            return;
        }

    err = pwm_set_pulse_dt(&LED1::green_pwm_led, STEP_SIZE);
    if (err) {
        printk("Error %d: solid green write failed\n", err);
        return;
    }

    err = pwm_set_pulse_dt(&LED1::red_pwm_led, STEP_SIZE);
    if (err) {
        printk("Error %d: solid red write failed\n", err);
        return;
    }
}

// LOW BATTERY INDICATION LED
void LED1::set_blue_white(bool low_battery) {
    int err;

    LOG_DBG("set LED to blue and white alternating");

    while (low_battery) {

        // Turn on blue, red, and green for white
        err = pwm_set_pulse_dt(&LED1::blue_pwm_led, STEP_SIZE);
            if (err != 0) {
                printk("Error %d: blue write failed\n",
                        err);
                return;
            }

        err = pwm_set_pulse_dt(&LED1::green_pwm_led, STEP_SIZE);
        if (err) {
            printk("Error %d: solid green write failed\n", err);
            return;
        }

        err = pwm_set_pulse_dt(&LED1::red_pwm_led, STEP_SIZE);
        if (err) {
            printk("Error %d: solid red write failed\n", err);
            return;
        }

        k_sleep(K_MSEC(10000));

        // Turn off red and green for blue only
        err = pwm_set_pulse_dt(&LED1::green_pwm_led, 0);
        if (err) {
            printk("Error %d: solid green write failed\n", err);
            return;
        }

        err = pwm_set_pulse_dt(&LED1::red_pwm_led, 0);
        if (err) {
            printk("Error %d: solid red write failed\n", err);
            return;
        }
    }
}

void LED1::init() {
    // Check that led's are ready
    if (!device_is_ready(LED1::red_pwm_led.dev) || !device_is_ready(LED1::green_pwm_led.dev) || !device_is_ready(LED1::blue_pwm_led.dev)) {
        LOG_ERR("Error: LED is not ready");
		return;
	}

    return;
}