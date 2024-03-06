#ifndef AFE_CONFIG_H
#define AFE_CONFIG_H

#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/led.h>

// TODO: protect in namespace
// AFE defines/device
#define AFE_SPI DT_NODELABEL(afespi)
#define SPI_CS_PIN 8
#define SPI_FREQUENCY_HZ 2000000

// AFE RESET
#define LED1_NODE DT_ALIAS(afereset)
#define LED1_PIN DT_GPIO_PIN(LED1_NODE, gpios)

struct ads1299_config {
	struct spi_dt_spec bus;
#if CONFIG_ADC_ASYNC
	k_thread_stack_t *stack;
#endif
	const struct gpio_dt_spec gpio_reset;
	const struct gpio_dt_spec gpio_data_ready;
};

#endif