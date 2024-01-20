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

#include <nrf.h>
#include <nrfx.h>

#include <stdlib.h>

#include "arm_math.h"
#include "DataAcquisitionThread.h"

LOG_MODULE_REGISTER(eegals_app_core, LOG_LEVEL_DBG);

#define LOG_DELAY_MS 1000

/** @brief Allow access to specific GPIOs for the network core.
 *
 * Function is executed very early during system initialization to make sure
 * that the network core is not started yet. More pins can be added if the
 * network core needs them.
 */
static int network_gpio_allow(void)
{
	// TBD
	// uint32_t start_pin = (IS_ENABLED(CONFIG_SOC_ENABLE_LFXO) ? 2 : 0);

	// /* Allow the network core to use all GPIOs. */
	// for (uint32_t i = start_pin; i < P0_PIN_NUM; i++) {
	// 	NRF_P0_S->PIN_CNF[i] = (GPIO_PIN_CNF_MCUSEL_NetworkMCU <<
	// 				GPIO_PIN_CNF_MCUSEL_Pos);
	// }

	// for (uint32_t i = 0; i < P1_PIN_NUM; i++) {
	// 	NRF_P1_S->PIN_CNF[i] = (GPIO_PIN_CNF_MCUSEL_NetworkMCU <<
	// 				GPIO_PIN_CNF_MCUSEL_Pos);
	// }


	return 0;
}

int main(void)
{
	LOG_INF("Hello world from %s", CONFIG_BOARD);

	DataAcquisitionThread::GetInstance().Initialize();

	init_ble_handler();

	while(1) {
		LOG_DBG("main thread up time: %u ms", k_uptime_get_32());
		k_msleep(LOG_DELAY_MS);
	}

	return 0;
}

// TBD
//SYS_INIT(network_gpio_allow, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_OBJECTS);
