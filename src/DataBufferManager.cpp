#include "DataBufferManager.h"

struct Semaphore DataBufferManager::eeg_buffer_semaphore;
eeg_sample DataBufferManager::dma_buffer[max_samples];
size_t DataBufferManager::buffer_index;

void DataBufferManager::Write() {

}

void DataBufferManager::Read() {

}

void DataBufferManager::spi_dma_setup() {
    // IF NOT SET UP, CONTINUE; ELSE ERR
    // // Configure SPI CS pin as GPIO output
    // const struct device *cs_dev = device_get_binding(DT_GPIO_LABEL(DT_NODELABEL(spi0), cs_gpios));
    // gpio_pin_configure(cs_dev, SPI_CS_PIN, GPIO_OUTPUT_ACTIVE);

    // // Configure SPI device
    // struct spi_config spi_cfg = {
    //     .frequency = 500000,  // Set your desired frequency
    //     .operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8),
    //     .cs = NULL,
    // };

    // // Set up SPI device
    // if (spi_configure(spi_dev, &spi_cfg) != 0) {
    //     printk("SPI configuration failed\n");
    //     return;
    // }

    // printk("SPI configured successfully\n");

}

void DataBufferManager::spi_sensor_read_and_transfer() {
    
}
