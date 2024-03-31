#pragma once
#include <zephyr/drivers/pwm.h>



namespace LED1 {
    const struct pwm_dt_spec red_pwm_led = PWM_DT_SPEC_GET(DT_ALIAS(redled));
    const struct pwm_dt_spec green_pwm_led = PWM_DT_SPEC_GET(DT_ALIAS(greenled));
    const struct pwm_dt_spec blue_pwm_led = PWM_DT_SPEC_GET(DT_ALIAS(blueled));

    void init();
    void set_blue();
    void set_yellow();
    void set_solid_green();
    void set_flash_red();
    void set_solid_red();
    void set_white();
    void set_blue_white(bool low_battery);
};
