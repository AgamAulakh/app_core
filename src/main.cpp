/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/init.h>
#include <nrf.h>
#include <nrfx.h>

#include <stdlib.h>

#include "ArmMatrixWrapper.h"
#include "DataAcquisitionThread.h"
#include "drivers/ads1299-x.h"

// temporary
#include <zephyr/drivers/pwm.h>

LOG_MODULE_REGISTER(eegals_app_core, LOG_LEVEL_DBG);

static const struct pwm_dt_spec blue_pwm_led = PWM_DT_SPEC_GET(DT_ALIAS(blueled));

#define LOG_DELAY_MS 5000
#define LED_PERIOD 1000
#define LED_OFF 0
#define LED_CHANNEL 0
#define LED_BLUE_PULSE_WIDTH 1000


int main(void)
{
	LOG_INF("Hello world from %s", CONFIG_BOARD);

	DataAcquisitionThread::GetInstance().Initialize();

	while(1) {
		LOG_DBG("main thread up time: %u ms", k_uptime_get_32());
		DataAcquisitionThread::GetInstance().SendMessage(
			DataAcquisitionThread::RESET_AFE
		);
		pwm_set_dt(&blue_pwm_led, LED_PERIOD, LED_BLUE_PULSE_WIDTH);
		k_msleep(LOG_DELAY_MS);

		pwm_set_dt(&blue_pwm_led, LED_PERIOD, LED_OFF);
		k_msleep(LOG_DELAY_MS);
	}

	return 0;
};
