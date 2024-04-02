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
#include <zephyr/smf.h>

extern "C" {
#include <ble_handler.h>
}

#include <state_machine.h>
#include <led_handler.h>

#include <nrf.h>
#include <nrfx.h>
#include <zephyr/logging/log.h>

#include <stdlib.h>

#include "ArmMatrixWrapper.h"
#include "DataAcquisitionThread.h"
#include "SignalProcessingThread.h"
#include "drivers/ads1299-x.h"

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
	LOG_INF("Hello world from %s", CONFIG_BOARD);
	TIBareMetalWrapper AFEWrapper;
	k_msleep(2500);
	AFEWrapper.Initialize();

	while(1) {
		// DataAcquisitionThread::GetInstance().SendMessage(
		// 	DataAcquisitionThread::READ_AFE_SAMPLE
		// );
		LOG_DBG("main thread up time: %u ms", k_uptime_get_32());
		
		pwm_set_dt(&blue_pwm_led, LED_PERIOD, LED_BLUE_PULSE_WIDTH);
		k_msleep(LOG_DELAY_MS);

		pwm_set_dt(&blue_pwm_led, LED_PERIOD, LED_OFF);
		k_msleep(LOG_DELAY_MS);

		AFEWrapper.RunInternalSquareWaveTest();
	}
	// // state_machine_init();
	// LED1::init();


	// DataAcquisitionThread::GetInstance().Initialize();
	// SignalProcessingThread::GetInstance().Initialize();

	// k_msleep(2500);

	// DataAcquisitionThread::GetInstance().SendMessage(
	// 	DataAcquisitionThread::INITIALIZE_AFE
	// );
	// DataAcquisitionThread::GetInstance().SendMessage(
	// 	DataAcquisitionThread::RUN_INTERNAL_SQUARE_WAVE_TEST_BIG_FAST
	// );
	// SignalProcessingThread::GetInstance().SendMessage(
	// 	SignalProcessingThread::START_PROCESSING
	// );

	// LOG_DBG("main thread waiting for sigproc done signal: %u ms", k_uptime_get_32());

	// if(SignalProcessingThread::done_flag.wait()){
	// 	DataAcquisitionThread::GetInstance().SendMessage(
	// 		DataAcquisitionThread::STOP_READING_AFE
	// 	);
	// }
	// // k_event_wait(&s_obj.sig_proc_complete, EVENT_SIG_PROC_COMPLETE, true, K_FOREVER);
	// // LOG_DBG("main thread received sigproc done signal: %u ms", k_uptime_get_32());

	// while(1) {
	// 	k_msleep(LOG_DELAY_MS);
	// 	LOG_DBG("main thread up time: %u ms", k_uptime_get_32());
	// }

	// return 0;
};
