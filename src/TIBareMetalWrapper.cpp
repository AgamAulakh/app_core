#include "TIBareMetalWrapper.h"
#include <zephyr/drivers/gpio.h>

// constants to be moved
constexpr uint8_t num_bytes_per_sample = 3 * (8 + 1);
constexpr uint16_t num_samples = 250;
constexpr uint16_t rx_buf_len = num_bytes_per_sample * num_samples;

// log level declaration
LOG_MODULE_REGISTER(TI_bare_metal_wrapper, LOG_LEVEL_DBG);

// Static fields
uint8_t TIBareMetalWrapper::master_counter = 0;
gpio_dt_spec TIBareMetalWrapper::afe_reset_spec = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, afereset_gpios);
gpio_dt_spec TIBareMetalWrapper::afe_drdy_spec = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, afedrdy_gpios);
gpio_dt_spec TIBareMetalWrapper::afe_indicate_spec = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, afeindicator_gpios);
gpio_callback TIBareMetalWrapper::afe_drdy_cb_data;
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

    // AFE DRDY
    err = gpio_pin_configure_dt(&afe_drdy_spec, GPIO_INPUT);
    if (err != 0) {
        LOG_ERR("COULD NOT CONFIGURE AFE DRDY AS GPIO INPUT");
    }

    err = gpio_pin_interrupt_configure_dt(&afe_drdy_spec, GPIO_INT_EDGE_FALLING);
    if (err != 0) {
        LOG_ERR("COULD NOT CONFIGURE AFE DRDY INTERRUPT");
    }

    // gpio_init_callback(&afe_drdy_cb_data, HandleDRDY, BIT(afe_drdy_spec.pin));
    // gpio_add_callback(afe_drdy_spec.port, &afe_drdy_cb_data);
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
        if (i % 27 == 0) {
            printk("\n");
        }
    }
    printk("\n");
    return;
};

void TIBareMetalWrapper::StartDMA(uint8_t rx[], uint16_t len) {
    struct spi_buf rx_buf = {
        .buf = rx,
        .len = len * sizeof(rx[0]),
    };
    const struct spi_buf_set rx_set = {
        .buffers = &rx_buf,
        .count = 1
    };

    // Start transaction
    LOG_INF("!!! read buffer start time: %u ms", k_uptime_get_32());
    int error = spi_read(spi_dev, &spi_cfg, &rx_set);
    if(error != 0){
        LOG_ERR("TIBareMetalWrapper::%s transceive error: %i", __FUNCTION__, error);
        return;
    }
    LOG_INF("!!! read buffer end time: %u ms", k_uptime_get_32());

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

void TIBareMetalWrapper::HandleDRDY(const device *dev, gpio_callback *cb, uint32_t pins){
    LOG_INF("TIBareMetalWrapper::%s got DRDY signal at %u ms", __FUNCTION__, k_uptime_get_32());
    // int32_t result = ADS1299_ReadAdc(&afe_driver);
};

// parent functions to override
void TIBareMetalWrapper::Initialize() {
    ADS1299_Init(&afe_driver);
    LOG_INF("finished ads1299 init");

    CheckID();
    CheckConfigRegs();
    CheckChannels();
    CheckBiasSensPReg();
    CheckBiasSensNReg();

    uint8_t turn_on_channel = 0x4A;
    turn_on_channel = turn_on_channel >> 1;
    ADS1299_SetCh1SetState(&afe_driver, turn_on_channel);
    ADS1299_SetCh2SetState(&afe_driver, turn_on_channel);
    ADS1299_SetCh3SetState(&afe_driver, turn_on_channel);
    ADS1299_SetCh4SetState(&afe_driver, turn_on_channel);
    ADS1299_SetCh5SetState(&afe_driver, turn_on_channel);
    ADS1299_SetCh6SetState(&afe_driver, turn_on_channel);
    ADS1299_SetCh7SetState(&afe_driver, turn_on_channel);
    ADS1299_SetCh8SetState(&afe_driver, turn_on_channel);

    uint8_t enable_bias = 0xFF;
    ADS1299_SetBiasSensNState(&afe_driver, enable_bias);
};

void TIBareMetalWrapper::Start() {
    CheckID();
    CheckConfigRegs();
    CheckChannels();
    CheckBiasSensPReg();
    CheckBiasSensNReg();

    // Start ADC conversion
    ADS1299_StartAdc(&afe_driver);
};

void TIBareMetalWrapper::ReadOneSample() {
    // starting adc
    CheckBiasSensPReg();
    CheckBiasSensNReg();

    // LOG_INF("enabling cont read");
    // ADS1299_EnableContRead(&afe_driver);

    // LOG_INF("enabled cont read");
    // uint8_t rx_buffer[rx_buf_len] = {0};
    // Read(rx_buffer, rx_buf_len);

    // LOG_INF("disabling cont read");
    // ADS1299_DisableContRead(&afe_driver);

    ADS1299_ReadAdc(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s ch1: %f, ch2: %f, ch3: %f, ch4: %f, ch5: %f, ch6: %f, ch7: %f, ch8: %f",
        __FUNCTION__,
        afe_driver.sample.ch1,
        afe_driver.sample.ch2,
        afe_driver.sample.ch3,
        afe_driver.sample.ch4,
        afe_driver.sample.ch5,
        afe_driver.sample.ch6,
        afe_driver.sample.ch7,
        afe_driver.sample.ch8
    );
};

void TIBareMetalWrapper::CheckID() {
    ADS1299_GetIdState(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s rev id: %u, dev id: %u, num channels: %u",
        __FUNCTION__,
        afe_driver.id.revId, // dont know/care
        afe_driver.id.devId, // should be 3
        afe_driver.id.nuCh   // should be 2 (for 8 channel ADS)
    );
};

void TIBareMetalWrapper::CheckConfigRegs() {
    ADS1299_GetConfig1State(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s config 1 daisyEn: %u, clkEn: %u, dataRate: %u",
        __FUNCTION__,
        afe_driver.config1.daisyEn,
        afe_driver.config1.clkEn,
        afe_driver.config1.dataRate
    );

    ADS1299_GetConfig2State(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s config 2 intCal: %u, calAmp: %u, calFreq: %u",
        __FUNCTION__,
        afe_driver.config2.intCal,
        afe_driver.config2.calAmp,
        afe_driver.config2.calFreq
    );

    ADS1299_GetConfig3State(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s config 3 pdRefBuf: %u, baisMeas: %u, biasRefInt: %u, pdBias: %u, biasLoffSens: %u, biasStat: %u",
        __FUNCTION__,
        afe_driver.config3.pdRefBuf,
        afe_driver.config3.biasMeas,
        afe_driver.config3.biasRefInt,
        afe_driver.config3.pdBias,
        afe_driver.config3.biasLoffSens,
        afe_driver.config3.biasStat
    );

    ADS1299_GetConfig4State(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s config 4 singleShot: %u, pdLoffComp: %u",
        __FUNCTION__,
        afe_driver.config4.singleShot,
        afe_driver.config4.pdLoffComp
    );
};

void TIBareMetalWrapper::CheckChannels(){
    ADS1299_GetCh1SetState(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s channel 1 pd1: %u, gain1: %u, srb2: %u, mux: %u",
        __FUNCTION__,
        afe_driver.ch1set.pd1,
        afe_driver.ch1set.gain1,
        afe_driver.ch1set.srb2,
        afe_driver.ch1set.mux1
    );
    ADS1299_GetCh2SetState(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s channel 2 pd2: %u, gain2: %u, srb2: %u, mux: %u",
        __FUNCTION__,
        afe_driver.ch2set.pd2,
        afe_driver.ch2set.gain2,
        afe_driver.ch2set.srb2,
        afe_driver.ch2set.mux2
    );
    ADS1299_GetCh3SetState(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s channel 3 pd3: %u, gain3: %u, srb2: %u, mux: %u",
        __FUNCTION__,
        afe_driver.ch3set.pd3,
        afe_driver.ch3set.gain3,
        afe_driver.ch3set.srb2,
        afe_driver.ch3set.mux3
    );
    ADS1299_GetCh4SetState(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s channel 4 pd4: %u, gain4: %u, srb2: %u, mux: %u",
        __FUNCTION__,
        afe_driver.ch4set.pd4,
        afe_driver.ch4set.gain4,
        afe_driver.ch4set.srb2,
        afe_driver.ch4set.mux4
    );
    ADS1299_GetCh5SetState(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s channel 5 pd5: %u, gain5: %u, srb2: %u, mux: %u",
        __FUNCTION__,
        afe_driver.ch5set.pd5,
        afe_driver.ch5set.gain5,
        afe_driver.ch5set.srb2,
        afe_driver.ch5set.mux5
    );
    ADS1299_GetCh6SetState(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s channel 6 pd6: %u, gain6: %u, srb2: %u, mux: %u",
        __FUNCTION__,
        afe_driver.ch6set.pd6,
        afe_driver.ch6set.gain6,
        afe_driver.ch6set.srb2,
        afe_driver.ch6set.mux6
    );
    ADS1299_GetCh7SetState(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s channel 7 pd7: %u, gain7: %u, srb2: %u, mux: %u",
        __FUNCTION__,
        afe_driver.ch7set.pd7,
        afe_driver.ch7set.gain7,
        afe_driver.ch7set.srb2,
        afe_driver.ch7set.mux7
    );
    ADS1299_GetCh8SetState(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s channel 8 pd8: %u, gain8: %u, srb2: %u, mux: %u",
        __FUNCTION__,
        afe_driver.ch8set.pd8,
        afe_driver.ch8set.gain8,
        afe_driver.ch8set.srb2,
        afe_driver.ch8set.mux8
    );
};

void TIBareMetalWrapper::CheckBiasSensPReg() {
    ADS1299_GetBiasSensPState(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s p1: %u, p2: %u, p3: %u, p4: %u, p5: %u, p6: %u, p7: %u, p8: %u",
        __FUNCTION__, 
        afe_driver.biassensp.biasP1,
        afe_driver.biassensp.biasP2,
        afe_driver.biassensp.biasP3,
        afe_driver.biassensp.biasP4,
        afe_driver.biassensp.biasP5,
        afe_driver.biassensp.biasP6,
        afe_driver.biassensp.biasP7,
        afe_driver.biassensp.biasP8
    );
};

void TIBareMetalWrapper::CheckBiasSensNReg() {
    ADS1299_GetBiasSensNState(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s p1: %u, p2: %u, p3: %u, p4: %u, p5: %u, p6: %u, p7: %u, p8: %u",
        __FUNCTION__, 
        afe_driver.biassensn.biasN1,
        afe_driver.biassensn.biasN2,
        afe_driver.biassensn.biasN3,
        afe_driver.biassensn.biasN4,
        afe_driver.biassensn.biasN5,
        afe_driver.biassensn.biasN6,
        afe_driver.biassensn.biasN7,
        afe_driver.biassensn.biasN8
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
