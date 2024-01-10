#include "CustomFrontEndWrapper.h"

const char *CustomFrontEndWrapper::spi_device_name = "ADS1299";
const struct device *CustomFrontEndWrapper::spi_dev;

CustomFrontEndWrapper::CustomFrontEndWrapper() {
    spi_dev = device_get_binding(SPI_DEV_NAME);
    if (!spi_dev) {
        // TODO: print on debug uart
        return;
    }
}

void CustomFrontEndWrapper::Initialize() {
    // // Set SPI to default settings
    // spi_cfg_t spi_config spi_cfg = {
    //     .operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8),
    //     .frequency = 500000,  // 500 kHz
    //     .slave = 0,
    // };

    // if (spi_configure(spi_dev, &spi_cfg) != 0) {
    //     // TODO: print on debug UART
    // }

    // DataBufferManager::spi_dma_setup(spi_dev);
}

void CustomFrontEndWrapper::Configure() {
    // TODO
}

eeg_sample CustomFrontEndWrapper::ReadData() {
    // TODO
}