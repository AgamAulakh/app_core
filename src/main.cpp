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
#include "led_handler.h"
#include "DataAcquisitionThread.h"
#include "SignalProcessingThread.h"
#include "drivers/ads1299.h"

// temporary
#include <zephyr/drivers/pwm.h>

LOG_MODULE_REGISTER(eegals_app_core, LOG_LEVEL_DBG);

static const struct pwm_dt_spec blue_pwm_led = PWM_DT_SPEC_GET(DT_ALIAS(blueled));

#define LOG_DELAY_MS 3000
#define LED_PERIOD 1000
#define LED_OFF 0
#define LED_CHANNEL 0
#define LED_BLUE_PULSE_WIDTH 1000


int main(void)
{
	// state_machine_init();
	LED1::init();

	DataAcquisitionThread::GetInstance().Initialize();
	SignalProcessingThread::GetInstance().Initialize();

	// DataAcquisitionThread::GetInstance().SendMessage(
	// 	DataAcquisitionThread::CHECK_AFE_REGISTERS
	// );

	// testing DAQ and sigproc together

	DataAcquisitionThread::GetInstance().SendMessage(
		DataAcquisitionThread::RUN_INTERNAL_SQUARE_WAVE_TEST_BIG_FAST
	);
	SignalProcessingThread::GetInstance().SendMessage(
		SignalProcessingThread::START_PROCESSING
	);

	// sleep for 10 seconds
	k_msleep(10000);

	// stop processing manually
	SignalProcessingThread::GetInstance().SendMessage(
		SignalProcessingThread::FORCE_STOP_PROCESSING
	);
	DataAcquisitionThread::GetInstance().SendMessage(
		DataAcquisitionThread::STOP_READING_AFE
	);

	while(1) {
		LOG_DBG("main thread up time: %u ms", k_uptime_get_32());
		k_msleep(LOG_DELAY_MS);
	}

	return 0;
};
