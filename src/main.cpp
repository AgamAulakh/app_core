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

#include <nrf.h>
#include <nrfx.h>
#include <zephyr/logging/log.h>

#include <stdlib.h>
#include "SigProcThread.h"

LOG_MODULE_REGISTER(app_core_main, LOG_LEVEL_INF);

#define LOG_DELAY_MS 1000

int main(void)
{
	SigProcThread::GetInstance().Initialize();


	
	//SigProcThread::GetInstance().SendMessage(SigProcThread::COMPUTE_BANDPOWER_RESULTS);
	SigProcThread::GetInstance().SendMessage(SigProcThread::COMPUTE_FFT_RESULTS);
   	while(1)
   	{
        //SigProcThread::GetInstance().SendMessage(SigProcThread::COMPUTE_FFT_RESULTS);
		//SigProcThread::GetInstance().SendMessage(SigProcThread::COMPUTE_POWER_RESULTS);
		//SigProcThread::GetInstance().SendMessage(SigProcThread::COMPUTE_BANDPOWER_RESULTS);
        k_msleep(1000);

    }

	return 0;
}
