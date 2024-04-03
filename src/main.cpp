/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>
#include <StateMachineThread.h>
#include <HIDThread.h>
#include <zephyr/init.h>
#include <HIDThread.h>
#include <nrf.h>
#include <nrfx.h>

#include <stdlib.h>

#include "ArmMatrixWrapper.h"
#include "led_handler.h"
#include "DataAcquisitionThread.h"
#include "SignalProcessingThread.h"
#include "drivers/ads1299.h"

// temporary
#include <zephyr/drivers/pwm.h>

LOG_MODULE_REGISTER(eegals_app_core, LOG_LEVEL_DBG);

// static const struct pwm_dt_spec blue_pwm_led = PWM_DT_SPEC_GET(DT_ALIAS(blueled));

#define LOG_DELAY_MS 3000
// #define LED_PERIOD 1000
// #define LED_OFF 0
// #define LED_CHANNEL 0
// #define LED_BLUE_PULSE_WIDTH 1000


int main(void)
{
	LOG_INF("Hello world from %s", CONFIG_BOARD);

	StateMachineThread::GetInstance().Initialize();
    HIDThread::GetInstance().Initialize();
	DataAcquisitionThread::GetInstance().Initialize();
	SignalProcessingThread::GetInstance().Initialize();

	while(1) {
		LOG_DBG("main thread Hello World");
		k_msleep(LOG_DELAY_MS);
	}

	return 0;
};
