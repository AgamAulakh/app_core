/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>
#include <StateMachineThread.h>
#include <zephyr/init.h>
#include <zephyr/sys/ring_buffer.h>

extern "C" {
#include <ble_handler.h>
}

#include <state_machine.h>
#include <led_handler.h>
#include "DataAcquisitionThread.h"
#include "SignalProcessingThread.h"
#include "ArmMatrixWrapper.h"
#include "Events.h"

LOG_MODULE_REGISTER(app_core_main, LOG_LEVEL_INF);
#define LOG_DELAY_MS 5000

int main(void)
{

	// state_machine_init();
	LED1::init();

	DataAcquisitionThread::GetInstance().Initialize();
	SignalProcessingThread::GetInstance().Initialize();

	k_msleep(2500);
	// DataAcquisitionThread::GetInstance().SendMessage(
	// 	DataAcquisitionThread::CHECK_AFE_REGISTERS
	// );

	// testing DAQ and sigproc together
	k_event_init(&s_obj.sig_proc_complete);

	DataAcquisitionThread::GetInstance().SendMessage(
		DataAcquisitionThread::RUN_FAKE_SAMPLES_DATA_BUFFER_TEST
	);
	SignalProcessingThread::GetInstance().SendMessage(
		SignalProcessingThread::START_PROCESSING
	);

	// LOG_DBG("main thread waiting for sigproc done signal: %u ms", k_uptime_get_32());
	// if(k_event_wait(&s_obj.sig_proc_complete, EVENT_SIG_PROC_COMPLETE, true, K_FOREVER)) {
	//     LOG_DBG("main thread received sigproc done signal: %u ms", k_uptime_get_32());
    // }

	while(1) {
		LOG_DBG("main thread up time: %u ms", k_uptime_get_32());
		k_msleep(LOG_DELAY_MS);
	}

	return 0;
};
