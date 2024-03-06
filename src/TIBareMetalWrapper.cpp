#include "TIBareMetalWrapper.h"

// log level declaration
LOG_MODULE_REGISTER(TI_bare_metal_wrapper, LOG_LEVEL_DBG);

device* afe_spi_global_device = const_cast<struct device *>(DEVICE_DT_GET(AFE_SPI));
// spi_dt_spec* afe_spi_bus = const_cast<struct spi_dt_spec*>(DT_BUS(AFE_SPI));

spi_config afe_spi_global_config = {
    SPI_FREQUENCY_HZ,
    (SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8)),
    0,
    0,
};

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct device *led_dev = DEVICE_DT_GET(LED1_NODE);

TIBareMetalWrapper::TIBareMetalWrapper() {

    LOG_INF("start ads1299 init");
    afe_driver = new ads1299_t();
    afe_driver->DelayMs = &DelayMs;
    afe_driver->DelayUs = &DelayUs;
    afe_driver->Transfer = &Transfer;
    afe_driver->SetCS = &SetCS;
    afe_driver->SetReset = &SetReset;
    afe_driver->SetStart = &SetStart;
    afe_driver->SetPWDN = &SetPWDN;

    LOG_INF("1 ads1299 init");
    if (!device_is_ready(afe_spi_global_device)) {
        LOG_ERR("TIBM::%s -- SPI device not ready!", __FUNCTION__);
        // todo: error handling
    }

    LOG_INF("3");


    if (!gpio_is_ready_dt(&led)) {
		return;
	}

	int ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
        LOG_ERR("cant config reset pin");

		return;
	}

    ADS1299_Init(afe_driver);
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
        LOG_ERR("TIBareMetalWrapper::%s SPI transfer failed with error %i", __FUNCTION__, ret);
    }
}

void TIBareMetalWrapper::SetReset(uint8_t state) {
    if (state) {
        int err_code = gpio_pin_set_dt(&led, 1);
        if (err_code != 0) {
            LOG_ERR("TIBareMetalWrapper::%s could not set afe reset", __FUNCTION__);
        }
    } else {
        int err_code = gpio_pin_set_dt(&led, 0);
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
    ADS1299_GetIdState(afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s revid: %d devid: %d numch: %d", __FUNCTION__, afe_driver->id.revId, afe_driver->id.devId, afe_driver->id.nuCh);
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
