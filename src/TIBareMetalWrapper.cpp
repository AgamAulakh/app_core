#include "TIBareMetalWrapper.h"
#include <zephyr/drivers/gpio.h>

// log level declaration
LOG_MODULE_REGISTER(TI_bare_metal_wrapper, LOG_LEVEL_DBG);

// Static fields
uint8_t TIBareMetalWrapper::master_counter = 0;
gpio_dt_spec TIBareMetalWrapper::afe_reset_spec = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, afereset_gpios);
gpio_dt_spec TIBareMetalWrapper::afe_indicate_spec = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, afeindicator_gpios);
k_poll_signal TIBareMetalWrapper::spi_done_sig = K_POLL_SIGNAL_INITIALIZER(spi_done_sig);
const device* TIBareMetalWrapper::spi_dev = 
    static_cast<const device*>(
        DEVICE_DT_GET(AFE_SPI)
    );
spi_config TIBareMetalWrapper::spi_cfg = {
    .frequency = AFE_SPI_FREQUENCY_HZ,
    .operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_MODE_CPOL | SPI_MODE_CPHA,
    .slave = 0,
    .cs = {.gpio = AFE_SPI_CS_DT_SPEC, .delay = 0},
};
ads1299_t TIBareMetalWrapper::afe_driver{
    &DelayMs, &DelayUs, &Transfer, &SetCS, &SetReset, &SetStart, &SetPWDN
};

TIBareMetalWrapper::TIBareMetalWrapper() {
    if(!device_is_ready(spi_dev)) {
        printk("SPI master device not ready!\n");
    }
    struct gpio_dt_spec spim_cs_gpio = AFE_SPI_CS_DT_SPEC;
    if(!device_is_ready(spim_cs_gpio.port)){
        printk("SPI master chip select device not ready!\n");
    }

    bool is_cs_gpio = spi_cs_is_gpio(&spi_cfg);
    LOG_INF("CS IS: %d", is_cs_gpio);

    // AFE RESET AND CHIP SELECT
    int err = gpio_pin_configure_dt(&afe_reset_spec, GPIO_OUTPUT_INACTIVE);
    if (err != 0) {
        LOG_ERR("COULD NOT CONFIGURE AFE RESET AS GPIO");
    }

    ADS1299_Init(&afe_driver);
    LOG_INF("finished ads1299 init");
};

// callbacks needed for driver
void TIBareMetalWrapper::SetCS(uint8_t state){
    if (state) {
        int err_code = gpio_pin_set_dt(&spi_cfg.cs.gpio, 1);
        err_code |= gpio_pin_set_dt(&afe_indicate_spec, 1);
        if (err_code != 0) {
            LOG_ERR("TIBareMetalWrapper::%s could not set chip select", __FUNCTION__);
        }
    } else {
        int err_code = gpio_pin_set_dt(&spi_cfg.cs.gpio, 0);
        err_code |= gpio_pin_set_dt(&afe_indicate_spec, 0);
        if (err_code != 0) {
            LOG_ERR("TIBareMetalWrapper::%s could not reset chip select", __FUNCTION__);
        }
    }
};

void TIBareMetalWrapper::DelayMs(uint32_t delay) {
    k_msleep(delay);
};

void TIBareMetalWrapper::DelayUs(uint32_t delay) {
    k_usleep(delay);
};

void TIBareMetalWrapper::Transfer(uint8_t tx[], uint8_t rx[], uint16_t len) {
    // NOTE: sometimes len < sizeof(tx) or sizeof(rx)
    // So we have to manually set length in spi_buf
    const struct spi_buf tx_buf = {
        .buf = tx,
        .len = len * sizeof(tx[0])
    };
    const struct spi_buf_set tx_set = {
        .buffers = &tx_buf,
        .count = 1
    };

    struct spi_buf rx_buf = {
        .buf = rx,
        .len = len * sizeof(rx[0]),
    };
    const struct spi_buf_set rx_set = {
        .buffers = &rx_buf,
        .count = 1
    };

    // Reset signal
    k_poll_signal_reset(&spi_done_sig);

    // Start transaction
    int error = spi_transceive_async(spi_dev, &spi_cfg, &tx_set, &rx_set, &spi_done_sig);
    if(error != 0){
        LOG_ERR("TIBareMetalWrapper::%s transceive error: %i", __FUNCTION__, error);
        return;
    }

    // Wait for the done signal to be raised and log the rx buffer
    int spi_result;
    unsigned int spi_signaled;
    do{
        k_poll_signal_check(&spi_done_sig, &spi_signaled, &spi_result);
    } while(spi_signaled == 0);

    printk("SPI RX:");
    for (size_t i = 0; i < len; i++) {
        printk(" 0x%.2x", rx[i]);
    }
    printk("\n");
    return;
};

void TIBareMetalWrapper::SetReset(uint8_t state) {
    if (state) {
        LOG_INF("IN AFE RESET -- SET HIGH");
        int err_code = gpio_pin_set_dt(&afe_reset_spec, 1);
        err_code |= gpio_pin_set_dt(&afe_indicate_spec, 1);

        if (err_code != 0) {
            LOG_ERR("TIBareMetalWrapper::%s could not set afe reset", __FUNCTION__);
        }
    } else {
        LOG_INF("IN AFE RESET -- SET LOW");
        int err_code = gpio_pin_set_dt(&afe_reset_spec, 0);
        err_code |= gpio_pin_set_dt(&afe_indicate_spec, 0);

        if (err_code != 0) {
            LOG_ERR("TIBareMetalWrapper::%s could not reset afe reset", __FUNCTION__);
        }
    }
};

void TIBareMetalWrapper::SetStart(uint8_t state){
    LOG_ERR("TIBareMetalWrapper::%s not implemented", __FUNCTION__);
};

void TIBareMetalWrapper::SetPWDN(uint8_t state){
    LOG_ERR("TIBareMetalWrapper::%s not implemented", __FUNCTION__);
};

// parent functions to override
void TIBareMetalWrapper::Initialize() {
};

void TIBareMetalWrapper::Start() {
    ADS1299_GetIdState(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s rev id: %u, dev id: %u, num channels: %u",
        __FUNCTION__,
        afe_driver.id.revId, // dont know/care
        afe_driver.id.devId, // should be 3
        afe_driver.id.nuCh   // should be 2 (for 8 channel ADS)
    );
};

void TIBareMetalWrapper::TestLoopbackSlave() {
    uint8_t tx[2] = {0};
    uint8_t rx[2] = {0};
    uint16_t len = 2;

    tx[0] = master_counter++;
    printk("SPI TX:");
    for (size_t i = 0; i < len; i++) {
        printk(" 0x%.2x", tx[i]);
    }
    printk("\n");

    Transfer(tx, rx, len);
};

void TIBareMetalWrapper::Wakeup() {
};

void TIBareMetalWrapper::Standby() {
};

void TIBareMetalWrapper::Reset() {
};

void TIBareMetalWrapper::Stop() {
};

void TIBareMetalWrapper::ReadData() {
};
