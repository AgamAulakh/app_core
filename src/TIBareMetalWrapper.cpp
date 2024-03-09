#include "TIBareMetalWrapper.h"
#include <zephyr/drivers/gpio.h>

// log level declaration
LOG_MODULE_REGISTER(TI_bare_metal_wrapper, LOG_LEVEL_DBG);

// Static fields
ads1299_t TIBareMetalWrapper::afe_driver{
    &DelayMs, &DelayUs, &Transfer, &SetCS, &SetReset, &SetStart, &SetPWDN
};

// Devicetree fields
device* led_reset_device = const_cast<struct device *>(DEVICE_DT_GET(LED1_RESET_DEV));
// gpio_dt_spec afe_reset_spec = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, afereset_gpios);
gpio_dt_spec afe_indicate_spec = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, afeindicator_gpios);

device* afe_spi_global_device = const_cast<struct device *>(DEVICE_DT_GET(AFE_SPI));
spi_config afe_spi_global_config = {
    SPI_FREQUENCY_HZ,
    (SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8)),
    0,
    0,
};

TIBareMetalWrapper::TIBareMetalWrapper() {
    if (!device_is_ready(afe_spi_global_device)) {
        LOG_ERR("TIBM::%s -- SPI device not ready!", __FUNCTION__);
        // todo: error handling
    }

    // int err = gpio_pin_configure_dt(&afe_reset_spec, GPIO_OUTPUT_INACTIVE);
    // if (err != 0) {
    //     LOG_ERR("COULD NOT CONFIGURE AFE RESET AS GPIO");
    // }

    // old LED-based gpio reset:
    if (led_reset_device == NULL || !device_is_ready(led_reset_device)) {
        LOG_ERR("AFE device doesnt exissst");
    }
    int err = gpio_pin_configure(led_reset_device, LED1_RESET_PIN, LED1_RESET_FLAGS);
    if (err != 0) {
        LOG_ERR("COULD NOT CONFIGURE AS GPIO");
    }

    ADS1299_Init(&afe_driver);
    LOG_INF("finished ads1299 init");
}

// callbacks needed for driver
void TIBareMetalWrapper::SetCS(uint8_t state){
    if (state) {
        int err_code = gpio_pin_set(afe_spi_global_device, SPI_CS_PIN, 1);
        if (err_code != 0) {
            LOG_ERR("TIBareMetalWrapper::%s could not set chip select", __FUNCTION__);
        }
    } else {
        int err_code = gpio_pin_set(afe_spi_global_device, SPI_CS_PIN, 0);
        if (err_code != 0) {
            LOG_ERR("TIBareMetalWrapper::%s could not reset chip select", __FUNCTION__);
        }
    }
}

void TIBareMetalWrapper::DelayMs(uint32_t delay) {
    k_msleep(delay);
}

void TIBareMetalWrapper::DelayUs(uint32_t delay) {
    k_usleep(delay);
}

void TIBareMetalWrapper::Transfer(uint8_t tx[], uint8_t rx[], uint16_t len) {
    // need to simultaneously read and write... ok makes sense
	const struct spi_buf tx_buf[] = {{
		.buf = tx,
		.len = len * sizeof(tx[0]),
	}};
	const struct spi_buf rx_buf[] = {{
		.buf = rx,
		.len = len * sizeof(tx[0]),
	}};
	const struct spi_buf_set tx_buf_set = {
		.buffers = tx_buf,
		.count = ARRAY_SIZE(tx_buf),
	};
	const struct spi_buf_set rx_buf_set = {
		.buffers = rx_buf,
		.count = ARRAY_SIZE(rx_buf),
	};

    int ret = spi_transceive(afe_spi_global_device, &afe_spi_global_config, &tx_buf_set, &rx_buf_set);
    if (ret) {
        LOG_ERR("TIBareMetalWrapper::%s SPI transfer failed with error %i", 
            __FUNCTION__,
            ret
        );
    }
}

void TIBareMetalWrapper::SetReset(uint8_t state) {
    if (state) {
        LOG_ERR("IN AFE RESET -- SET HIGH");
        // old LED-based gpio reset:
        int err_code = gpio_pin_set(led_reset_device, LED1_RESET_PIN, 1);
        // int err_code = gpio_pin_set_dt(&afe_reset_spec, 1);
        err_code |= gpio_pin_set_dt(&afe_indicate_spec, 1);

        if (err_code != 0) {
            LOG_ERR("TIBareMetalWrapper::%s could not set afe reset", __FUNCTION__);
        }
    } else {
        LOG_ERR("IN AFE RESET -- SET LOW");
        // old LED-based gpio reset:
        int err_code = gpio_pin_set(led_reset_device, LED1_RESET_PIN, 0);
        // int err_code = gpio_pin_set_dt(&afe_reset_spec, 0);
        err_code |= gpio_pin_set_dt(&afe_indicate_spec, 0);

        if (err_code != 0) {
            LOG_ERR("TIBareMetalWrapper::%s could not reset afe reset", __FUNCTION__);
        }
    }
}

void TIBareMetalWrapper::SetStart(uint8_t state){
    LOG_ERR("TIBareMetalWrapper::%s not implemented", __FUNCTION__);
}

void TIBareMetalWrapper::SetPWDN(uint8_t state){
    LOG_ERR("TIBareMetalWrapper::%s not implemented", __FUNCTION__);
}

// parent functions to override
void TIBareMetalWrapper::Initialize() {
}

void TIBareMetalWrapper::Start() {
    ADS1299_GetIdState(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s rev id: %u, dev id: %u, num channels: %u",
        __FUNCTION__,
        afe_driver.id.revId, // dont know/care
        afe_driver.id.devId, // should be 3
        afe_driver.id.nuCh   // should be 2 (for 8 channel ADS)
    );
}

void TIBareMetalWrapper::Wakeup() {
}

void TIBareMetalWrapper::Standby() {
}

void TIBareMetalWrapper::Reset() {
}

void TIBareMetalWrapper::Stop() {
}

void TIBareMetalWrapper::ReadData() {
}
