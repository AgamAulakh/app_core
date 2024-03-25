/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/init.h>
#include <nrf.h>
#include <nrfx.h>

#include <stdlib.h>

#include "HDIThread.h"
#include "arm_math.h"

int main(void)
{
	/* Power off RAM and suspend CPU */
	// disable_ram_and_wfi(&NRF_VMC->RAM[0].POWER,
	// 		    &NRF_VMC->RAM[ARRAY_SIZE(NRF_VMC->RAM) - 1].POWER);

    HDIThread::GetInstance().Initialize();

    while(1) {
        HDIThread::GetInstance().SendMessage(
            HDIThread::DISPLAY_HELLO_WORLD
        );
        k_msleep(1000);
    }

	return 0;
}