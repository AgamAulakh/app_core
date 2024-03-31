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

	DataAcquisitionThread::GetInstance().Initialize();
	SignalProcessingThread::GetInstance().Initialize();

	k_msleep(2500);
	DataAcquisitionThread::GetInstance().SendMessage(
		DataAcquisitionThread::CHECK_AFE_REGISTERS
	);

	// testing DAQ and sigproc together
	DataAcquisitionThread::GetInstance().SendMessage(
		DataAcquisitionThread::START_READING_AFE
	);
	SignalProcessingThread::GetInstance().SendMessage(
		SignalProcessingThread::START_PROCESSING
	);

	k_msleep(10000);

	// if sigproc stop:
	// 	DataAcquisitionThread::GetInstance().SendMessage(
	// 		DataAcquisitionThread::STOP_READING_AFE
	// 	);

	while(1) {
		LOG_DBG("main thread up time: %u ms", k_uptime_get_32());
		k_msleep(LOG_DELAY_MS);

		// SignalProcessingThread::GetInstance().SendMessage(
		// 	SignalProcessingThread::COMPUTE_DEBUG_FFT_RESULTS
		// );
	}

	return 0;
};
