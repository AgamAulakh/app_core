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

#include "arm_math.h"
#include "DataAcquisitionThread.h"

LOG_MODULE_REGISTER(eegals_app_core, LOG_LEVEL_DBG);

#define LOG_DELAY_MS 1000

int main(void)
{
	LOG_INF("Hello world from %s", CONFIG_BOARD);

	DataAcquisitionThread::GetInstance().Initialize();

	while(1) {
		LOG_DBG("main thread up time: %u ms", k_uptime_get_32());
		k_msleep(LOG_DELAY_MS);
	}

	return 0;
}
