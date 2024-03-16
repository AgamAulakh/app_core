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
#include <zephyr/init.h>

extern "C" {
#include <ble_handler.h>
}

#include <state_machine.h>
#include <led_handler.h>
#include "DataAcquisitionThread.h"

#include <nrf.h>
#include <nrfx.h>
#include <zephyr/logging/log.h>

#include <stdlib.h>
#include "SignalProcessingThread.h"

LOG_MODULE_REGISTER(app_core_main, LOG_LEVEL_INF);
#include "ArmMatrixWrapper.h"
#include "DataAcquisitionThread.h"

#define LOG_DELAY_MS 5000

int main(void)
{

	state_machine_init();

	LED1::init();
	// DataAcquisitionThread::GetInstance().Initialize();
	SignalProcessingThread::GetInstance().Initialize();

	// DataAcquisitionThread::GetInstance().SendMessage(
	// 	DataAcquisitionThread::CHECK_AFE_ID
	// );

	TIBareMetalWrapper AFEWrapper;
	k_msleep(2500);
	AFEWrapper.Initialize();

	while(1) {
		// DataAcquisitionThread::GetInstance().SendMessage(
		// 	DataAcquisitionThread::READ_AFE_SAMPLE
		// );
		LOG_DBG("main thread up time: %u ms", k_uptime_get_32());
		k_msleep(LOG_DELAY_MS);

		SignalProcessingThread::GetInstance().SendMessage(
			SignalProcessingThread::COMPUTE_FFT_RESULTS
		);

		AFEWrapper.RunInternalSquareWaveTest();
	}

	return 0;
};
