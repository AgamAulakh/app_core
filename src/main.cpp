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

#include "arm_math.h"

#define ASSERT_MSG_BUFFER_ALLOC_FAILED	"buffer allocation failed"
#define ASSERT_MSG_SNR_LIMIT_EXCEED		"signal-to-noise ratio " \
										"error limit exceeded"

/** @brief Power OFF entire RAM and suspend CPU forever.
 *
 * Function operates only on `register` variables, so GCC will not use
 * RAM after function prologue. This is also true even with optimizations
 * disabled. Interrupts are disabled to make sure that they will never
 * be executed when RAM is powered off.
 *
 * @param reg_begin  Address to `POWER` register of the first NRF_VMC->RAM item
 * @param reg_last   Address to `POWER` register of the last NRF_VMC->RAM item
 */

#define SNR_ERROR_THRESH	((float32_t)120)
constexpr int32_t sigproc_buf_num_int = 8 * 1000 * 5;
constexpr int32_t afe_buf_num_int = 8 * 1000 * 1;

static float32_t test_buffer_sigproc [sigproc_buf_num_int] = { 0 };
static float32_t test_buffer_afe [afe_buf_num_int] = { 0 };

void disable_ram_and_wfi(register volatile uint32_t *reg_begin,
			 register volatile uint32_t *reg_last)
{
	__disable_irq();

	do {
		*reg_begin = 0;
		reg_begin += sizeof(NRF_VMC->RAM[0]) / sizeof(reg_begin[0]);
	} while (reg_begin <= reg_last);

	__DSB();
	do {
		__WFI();
	} while (1);
}

/**
 * NOTE: SHOULD NOT USE MALLOC OR USE SAFER ZEPHYR METHOD
 * NOTE: USE ZEPHYR SAFE ASSERT
*/
static void test_arm_rfft_f32_real_backend(
	bool inverse, const uint32_t *input, const uint32_t *ref,
	size_t length)
{
	arm_rfft_fast_instance_f32 inst;
	float32_t *output, *scratch;

	/* Initialise instance */
	arm_rfft_fast_init_f32(&inst, length);

	/* Allocate output buffer */
	output = (float32_t*)malloc(length * sizeof(float32_t));
	// zassert_not_null(output, ASSERT_MSG_BUFFER_ALLOC_FAILED);

	scratch = (float32_t*)calloc(length + 2, sizeof(float32_t)); /* see #24701 */
	// zassert_not_null(scratch, ASSERT_MSG_BUFFER_ALLOC_FAILED);

	/* Load data in place */
	memcpy(scratch, input, length * sizeof(float32_t));

	/* Run test function */
	arm_rfft_fast_f32(&inst, scratch, output, inverse);

	/* Validate output */
	// zassert_true(
	// 	test_snr_error_f32(length, output, (float32_t *)ref,
	// 		SNR_ERROR_THRESH),
	// 	ASSERT_MSG_SNR_LIMIT_EXCEED);

	/* Free output buffer */
	free(output);
	free(scratch);
}

static void test_arm_rfft_f32_real(const uint32_t *input, const uint32_t *ref, size_t length) {
	test_arm_rfft_f32_real_backend(false, input, ref, length);
}

int main(void)
{
	/* Power off RAM and suspend CPU */
	// disable_ram_and_wfi(&NRF_VMC->RAM[0].POWER,
	// 		    &NRF_VMC->RAM[ARRAY_SIZE(NRF_VMC->RAM) - 1].POWER);

	test_buffer_sigproc[0] = 1;
	test_buffer_afe[0] = 1;

	size_t fft_length = 100;
	uint32_t fft_input[fft_length] = { 0 };
	uint32_t fft_output[fft_length] = { 0 };

	test_arm_rfft_f32_real(fft_input, fft_output, fft_length);

	return 0;
}

/** @brief Allow access to specific GPIOs for the network core.
 *
 * Function is executed very early during system initialization to make sure
 * that the network core is not started yet. More pins can be added if the
 * network core needs them.
 */
static int network_gpio_allow(void)
{

	/* When the use of the low frequency crystal oscillator (LFXO) is
	 * enabled, do not modify the configuration of the pins P0.00 (XL1)
	 * and P0.01 (XL2), as they need to stay configured with the value
	 * Peripheral.
	 */
	uint32_t start_pin = (IS_ENABLED(CONFIG_SOC_ENABLE_LFXO) ? 2 : 0);

	/* Allow the network core to use all GPIOs. */
	for (uint32_t i = start_pin; i < P0_PIN_NUM; i++) {
		NRF_P0_S->PIN_CNF[i] = (GPIO_PIN_CNF_MCUSEL_NetworkMCU <<
					GPIO_PIN_CNF_MCUSEL_Pos);
	}

	for (uint32_t i = 0; i < P1_PIN_NUM; i++) {
		NRF_P1_S->PIN_CNF[i] = (GPIO_PIN_CNF_MCUSEL_NetworkMCU <<
					GPIO_PIN_CNF_MCUSEL_Pos);
	}


	return 0;
}

SYS_INIT(network_gpio_allow, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_OBJECTS);
