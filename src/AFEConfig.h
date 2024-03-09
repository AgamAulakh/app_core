#ifndef AFE_CONFIG_H
#define AFE_CONFIG_H

#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/led.h>

// AFE SPI
#define AFE_SPI DT_NODELABEL(afespi)
#define SPI_CS_DEV DT_PHANDLE(AFE_SPI, cs_gpios)
#define SPI_CS_PIN DT_PHA(AFE_SPI, cs_gpios, pin)
#define SPI_FREQUENCY_HZ 2000000

// AFE RESET PIN
#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)
#define AFE_RESET_NODE DT_NODELABEL(ZEPHYR_USER_NODE)
#define AFE_RESET_DEV DT_PROP(ZEPHYR_USER_NODE, afereset_gpios)

#define AFE_INDICATE_NODE DT_NODELABEL(ZEPHYR_USER_NODE)
#define AFE_INDICATE_DEV DT_PROP(ZEPHYR_USER_NODE, afeindicator_gpios)

// AFE RESET LED METHOD
#define LED1_RESET DT_NODELABEL(led1)
#define LED1_RESET_DEV DT_PHANDLE(LED1_RESET, gpios)
#define LED1_RESET_PIN DT_PHA(LED1_RESET, gpios, pin)
#define LED1_RESET_FLAGS DT_PHA(LED1_RESET, gpios, flags)

// EXTRA: doesn't work, isnt being used
struct ads1299_config {
	struct spi_dt_spec bus;
#if CONFIG_ADC_ASYNC
	k_thread_stack_t *stack;
#endif
	const struct gpio_dt_spec gpio_reset;
	const struct gpio_dt_spec gpio_data_ready;
};

#endif