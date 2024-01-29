#include "TIFrontEndWrapper.h"
// log level declaration
LOG_MODULE_REGISTER(TI_front_end_wrapper, LOG_LEVEL_DBG);

TIFrontEndWrapper::TIFrontEndWrapper() :
    afe_spi_device(const_cast<struct device *>(DEVICE_DT_GET(AFE_SPI))),
    afe_spi_config{ADS1299_SPI_FREQUENCY_HZ, (SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8)), 0},
    afe_driver(afe_spi_device, &afe_spi_config)
{
    if (!device_is_ready(afe_spi_device)) {
        LOG_ERR("TIFE::%s -- SPI device not ready!", __FUNCTION__);
        // todo: error handling
    }
}

void TIFrontEndWrapper::Initialize() {
    // set up AFE
    afe_driver.ads1299_powerup_reset();
    afe_driver.ads1299_check_id();
    afe_driver.ads1299_init_regs();
}

void TIFrontEndWrapper::Start() {
    afe_driver.ads1299_start_rdatac();
}

void TIFrontEndWrapper::Wakeup() {
    afe_driver.ads1299_wake();
}

void TIFrontEndWrapper::Standby() {
    afe_driver.ads1299_standby();
}

void TIFrontEndWrapper::Reset() {
    afe_driver.ads1299_powerup_reset();
}

void TIFrontEndWrapper::Stop() {
    afe_driver.ads1299_stop_rdatac();
}

void TIFrontEndWrapper::ReadData() {
    int32_t channel_1, channel_2, channel_3, channel_4;

    afe_driver.get_eeg_voltage_samples(&channel_1, &channel_2, &channel_3, &channel_4);
	LOG_DBG("TIFE::%s -- Read samples: %d, %d, %d, %d", __FUNCTION__, channel_1, channel_2, channel_3, channel_4);
}