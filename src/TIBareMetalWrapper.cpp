#include "TIBareMetalWrapper.h"
#include <zephyr/drivers/gpio.h>

// log level declaration
LOG_MODULE_REGISTER(TI_bare_metal_wrapper, LOG_LEVEL_DBG);
k_work_q TIBareMetalWrapper::dma_work_queue;
K_WORK_DEFINE(dma_work, TIBareMetalWrapper::DMAWorkHandler);
K_WORK_DEFINE(dma_cleanup, TIBareMetalWrapper::DMACleanUpHandler);

// Static fields
uint8_t TIBareMetalWrapper::master_counter = 0;
bool TIBareMetalWrapper::is_adc_on = false;
bool TIBareMetalWrapper::is_test_on = false;
bool TIBareMetalWrapper::is_small_wave = false;
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
    &DelayMs, &DelayUs, &Transfer, &Read, &SetCS, &SetReset, &SetStart, &SetPWDN
};

TIBareMetalWrapper::TIBareMetalWrapper() {
    if(!device_is_ready(spi_dev)) {
        LOG_ERR("\n\nTIBareMetalWrapper::%s SPI master device not ready!\n\n", __FUNCTION__);
    }
    struct gpio_dt_spec spim_cs_gpio = AFE_SPI_CS_DT_SPEC;
    if(!device_is_ready(spim_cs_gpio.port)){
        LOG_ERR("\n\nTIBareMetalWrapper::%s SPI master chip select device not ready!\n\n", __FUNCTION__);
    }

    bool is_cs_gpio = spi_cs_is_gpio(&spi_cfg);
    LOG_INF("TIBareMetalWrapper::%s CS IS: %d", __FUNCTION__, is_cs_gpio);

    // AFE RESET AND CHIP SELECT
    int err = gpio_pin_configure_dt(&afe_reset_spec, GPIO_OUTPUT_INACTIVE);
    if (err != 0) {
        LOG_ERR("\n\nTIBareMetalWrapper::%s COULD NOT CONFIGURE AFE RESET AS GPIO\n\n", __FUNCTION__);
    }

    // AFE DRDY
    err = gpio_pin_configure_dt(&afe_drdy_spec, GPIO_INPUT);
    if (err != 0) {
        LOG_ERR("\n\nTIBareMetalWrapper::%s COULD NOT CONFIGURE AFE DRDY AS GPIO INPUT\n\n", __FUNCTION__);
    }

    err = gpio_pin_interrupt_configure_dt(&afe_drdy_spec, GPIO_INT_EDGE_FALLING);
    if (err != 0) {
        LOG_ERR("\n\nTIBareMetalWrapper::%s COULD NOT CONFIGURE AFE DRDY INTERRUPT\n\n", __FUNCTION__);
    }

    gpio_init_callback(&afe_drdy_cb_data, HandleDRDY, BIT(afe_drdy_spec.pin));

    //*************************** Hard reset KILLS AFE ***************************//
    ADS1299_Init(&afe_driver);
    //*************************** Hard reset KILLS AFE ***************************//
    LOG_INF("TIBareMetalWrapper::%s finished ads1299 init", __FUNCTION__);

    // Cycle start/stop ADC
    StartADC();
    StopADC();

    // Setup register states internal to driver:
    CheckID();
    CheckConfigRegs();
    CheckChannels();
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
    printk("SPI TX:");
    Utils::PrintBuffer(tx, len);

    const struct spi_buf tx_buf = {
        .buf = tx,
        .len = len * sizeof(tx[0]),
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

    Utils::ShiftRightLogicalBuffer(rx, len);
    printk("SPI RX:");
    Utils::PrintBuffer(rx, len);
    return;
};

void TIBareMetalWrapper::Read(uint8_t rx[], uint16_t len) {
    // NOTE: sometimes len < sizeof(tx) or sizeof(rx)
    // So we have to manually set length in spi_buf
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
    int error = spi_read_async(spi_dev, &spi_cfg, &rx_set, &spi_done_sig);
    if(error != 0){
        LOG_ERR("TIBareMetalWrapper::%s read error: %i", __FUNCTION__, error);
        return;
    }

    // Wait for the done signal to be raised and log the rx buffer
    int spi_result;
    unsigned int spi_signaled;
    do{
        k_poll_signal_check(&spi_done_sig, &spi_signaled, &spi_result);
    } while(spi_signaled == 0);

    Utils::ShiftRightLogicalBuffer(rx, len);
    // printk("READ SPI RX:");
    // Utils::PrintBuffer(rx, len);
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
    // write application-specific settings
    ADS1299_SetConfig1State(&afe_driver, ADS1299_CONFIG1_SETUP_ARDUINO_250);
    ADS1299_SetConfig2State(&afe_driver, ADS1299_CONFIG2_SETUP_TEST_d004V_d9HZ);
    ADS1299_SetConfig3State(&afe_driver, ADS1299_CONFIG3_SETUP_REF_BIAS);
    ADS1299_SetConfig4State(&afe_driver, ADS1299_CONFIG4_SETUP_LOFF_S_DIS);

    uint8_t channel_state = ADS1299_CH_N_SET_SETUP_NO
                            | ADS1299_CH_N_SET_SETUP_GAIN_4
                            | ADS1299_CH_N_SET_SETUP_SRB2_CL
                            | ADS1299_CH_N_SET_SETUP_MUX_NEI;
    ADS1299_SetCh1SetState(&afe_driver, channel_state);
    ADS1299_SetCh2SetState(&afe_driver, channel_state);
    ADS1299_SetCh3SetState(&afe_driver, channel_state);
    ADS1299_SetCh4SetState(&afe_driver, channel_state);
    ADS1299_SetCh5SetState(&afe_driver, channel_state);
    ADS1299_SetCh6SetState(&afe_driver, channel_state);
    ADS1299_SetCh7SetState(&afe_driver, channel_state);
    ADS1299_SetCh8SetState(&afe_driver, channel_state);

    ADS1299_SetBiasSensPState(&afe_driver, ADS1299_BIAS_SENSX_ALL_OFF);
    ADS1299_SetBiasSensNState(&afe_driver, ADS1299_BIAS_SENSX_ALL_ON);

    ADS1299_SetLoffSensPState(&afe_driver, ADS1299_LOFF_SENSX_ALL_OFF);
    ADS1299_SetLoffSensNState(&afe_driver, ADS1299_LOFF_SENSX_ALL_OFF);

    // check all registers were updated:
    CheckAllRegisters();

    DataBufferManager::Initialize();
};

void TIBareMetalWrapper::RunInputShortTest() {
    if (is_adc_on) { StopADC(); }

    LOG_DBG("TIBareMetalWrapper::%s starting input short test",__FUNCTION__);
    // NOTE: should be used after hard reset
    // set internal reference for power on test
    ADS1299_SetConfig3State(&afe_driver, ADS1299_CONFIG3_SETUP_REFBUF);

    // set deviec for DR = Fmod / 4096
    ADS1299_SetConfig1State(&afe_driver, ADS1299_CONFIG1_SETUP_DEFAULT);
    ADS1299_SetConfig2State(&afe_driver, ADS1299_CONFIG2_SETUP_DEFAULT);

    // set all channels to input short
    ADS1299_SetCh1SetState(&afe_driver, ADS1299_CH_N_SET_SETUP_MUX_IS);
    ADS1299_SetCh2SetState(&afe_driver, ADS1299_CH_N_SET_SETUP_MUX_IS);
    ADS1299_SetCh3SetState(&afe_driver, ADS1299_CH_N_SET_SETUP_MUX_IS);
    ADS1299_SetCh4SetState(&afe_driver, ADS1299_CH_N_SET_SETUP_MUX_IS);
    ADS1299_SetCh5SetState(&afe_driver, ADS1299_CH_N_SET_SETUP_MUX_IS);
    ADS1299_SetCh6SetState(&afe_driver, ADS1299_CH_N_SET_SETUP_MUX_IS);
    ADS1299_SetCh7SetState(&afe_driver, ADS1299_CH_N_SET_SETUP_MUX_IS);
    ADS1299_SetCh8SetState(&afe_driver, ADS1299_CH_N_SET_SETUP_MUX_IS);

    // verify all registers have been set
    CheckConfigRegs();
    CheckChannels();

    // ReadContinuous: activate converstion, DRDY should toggle at Fclk/8192
    // ReadContinuous: put device back in RDATAC mode
    // look for DRDY and issue 24 + 8x24 SCLKs
    // DRDY toggles at 2000000 / 8192 ~= 245
    // should be approx 250 samples
    ReadContinuous();
    // stop continuous read mode

    LOG_DBG("TIBareMetalWrapper::%s end",__FUNCTION__);
};

void TIBareMetalWrapper::RunInternalSquareWaveTest(bool is_small_wave) {
    if (is_adc_on) { StopADC(); }

    LOG_DBG("TIBareMetalWrapper::%s starting square wave test",__FUNCTION__);
    // NOTE: should be used after hard reset
    // set internal reference for power on test
    ADS1299_SetConfig3State(&afe_driver, ADS1299_CONFIG3_SETUP_REFBUF);

    if (is_small_wave) {
        // set devicc for DR = Fmod / 4096
        ADS1299_SetConfig2State(&afe_driver, ADS1299_CONFIG2_SETUP_TEST_d002V_1d9HZ);
    }
    else {
        // set device for DR = Fmod / 4096
        ADS1299_SetConfig2State(&afe_driver, ADS1299_CONFIG2_SETUP_TEST_d004V_d9HZ);
    }

    // set deviec for DR = Fmod / 4096
    ADS1299_SetConfig1State(&afe_driver, ADS1299_CONFIG1_SETUP_DEFAULT);
    // enable internal test signals
    // ADS1299_SetConfig2State(&afe_driver, ADS1299_CONFIG2_SETUP_TEST_d002V_d9HZ);

    // set all channels read test signal
    ADS1299_SetCh1SetState(&afe_driver, ADS1299_CH_N_SET_SETUP_MUX_TEST);
    ADS1299_SetCh2SetState(&afe_driver, ADS1299_CH_N_SET_SETUP_MUX_TEST);
    ADS1299_SetCh3SetState(&afe_driver, ADS1299_CH_N_SET_SETUP_MUX_TEST);
    ADS1299_SetCh4SetState(&afe_driver, ADS1299_CH_N_SET_SETUP_MUX_TEST);
    ADS1299_SetCh5SetState(&afe_driver, ADS1299_CH_N_SET_SETUP_MUX_TEST);
    ADS1299_SetCh6SetState(&afe_driver, ADS1299_CH_N_SET_SETUP_MUX_TEST);
    ADS1299_SetCh7SetState(&afe_driver, ADS1299_CH_N_SET_SETUP_MUX_TEST);
    ADS1299_SetCh8SetState(&afe_driver, ADS1299_CH_N_SET_SETUP_MUX_TEST);

    // verify all registers have been set
    CheckConfigRegs();
    CheckChannels();

    // set test flag
    is_test_on = true;

    // clear DMA buffer
    DataBufferManager::Initialize();
    DataBufferManager::ResetBuffer();    

    // ReadContinuous: activate converstion, DRDY should toggle at Fclk/8192
    // ReadContinuous: put device back in RDATAC mode
    // look for DRDY and issue 24 + 8x24 SCLKs
    // DRDY toggles at 2000000 / 8192 ~= 245
    // should be approx 250 samples
    ReadContinuous();

    LOG_DBG("TIBareMetalWrapper::%s end",__FUNCTION__);
};

// Controlled by State Machine
void TIBareMetalWrapper::Start() {
    // defensive check agaisnt double-starting test:
    if (is_test_on){
        LOG_ERR("TIBareMetalWrapper::%s Test Is ALREADY Running!", __FUNCTION__);
        return;
    }

    is_test_on = true;

    // Officially start sampling afe with no end in sight
    if (is_adc_on) { StopADC(); }
    if (afe_driver.config4.singleShot) { ConfigContinuousConversion(); }

    // clear DMA buffer
    DataBufferManager::Initialize();
    DataBufferManager::ResetBuffer();

    // Write setting first, then start adc conversion
    LOG_INF("TIBareMetalWrapper::%s enabling continuous read", __FUNCTION__);

    // send RDATAC command, then start adc conversion
    ADS1299_EnableContRead(&afe_driver);
    StartADC();

    // run our own DMA handler
    gpio_init_callback(&afe_drdy_cb_data, HandleDRDY, BIT(afe_drdy_spec.pin));
    gpio_add_callback(afe_drdy_spec.port, &afe_drdy_cb_data);
};

void TIBareMetalWrapper::Stop() {
    // defensive check agaisnt double-starting test:
    if (!is_test_on){
        LOG_ERR("TIBareMetalWrapper::%s Test Is NOT Running!", __FUNCTION__);
        return;
    }

    is_test_on = false;

    // Officially stop sampling afe
    k_work_submit(&dma_cleanup);
};

void TIBareMetalWrapper::HandleDRDY(const device *dev, gpio_callback *cb, uint32_t pins){
    k_work_submit(&dma_work);
};

void TIBareMetalWrapper::StartADC() {
    // Start ADC conversion
    LOG_INF("TIBareMetalWrapper::%s Starting ADC Conversion", __FUNCTION__);
    ADS1299_StartAdc(&afe_driver);
    is_adc_on = true;
};

void TIBareMetalWrapper::StopADC() {
    // STOP ADC conversion
    LOG_INF("TIBareMetalWrapper::%s Stopping ADC Conversion", __FUNCTION__);
    ADS1299_StopAdc(&afe_driver);
    is_adc_on = false;
}


void TIBareMetalWrapper::ConfigSingleShotConversion() {
    // NOTE: lead off is disabled!
    LOG_INF("TIBareMetalWrapper::%s Config 4 set to singleshot Conversion", __FUNCTION__);
    ADS1299_SetConfig4State(&afe_driver, ADS1299_CONFIG4_SETUP_LOFF_S_DIS);

    // read afe to make sure setting is correct
    ADS1299_GetConfig4State(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s config 4 singleShot: %u, pdLoffComp: %u",
        __FUNCTION__,
        afe_driver.config4.singleShot,
        afe_driver.config4.pdLoffComp
    );
};

void TIBareMetalWrapper::ConfigContinuousConversion() {
    // NOTE: lead off is disabled!
    LOG_INF("TIBareMetalWrapper::%s Config 4 set to continuous Conversion", __FUNCTION__);
    ADS1299_SetConfig4State(&afe_driver, ADS1299_CONFIG4_SETUP_LOFF_C_DIS);

    // read afe to make sure setting is correct
    ADS1299_GetConfig4State(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s config 4 singleShot: %u, pdLoffComp: %u",
        __FUNCTION__,
        afe_driver.config4.singleShot,
        afe_driver.config4.pdLoffComp
    );
};

void TIBareMetalWrapper::ReadOneSample() {
    if (!is_adc_on) { StartADC(); }
    if (!afe_driver.config4.singleShot) { ConfigSingleShotConversion(); }

    ADS1299_ReadAdcRegister(&afe_driver);
    PrintCurrentSample();
};

void TIBareMetalWrapper::ReadContinuous() {
    if (is_adc_on) { StopADC(); }
    if (afe_driver.config4.singleShot) { ConfigContinuousConversion(); }

    // Write setting first, then start adc conversion
    LOG_INF("TIBareMetalWrapper::%s enabling continuous read", __FUNCTION__);

    // send RDATAC command, then start adc conversion
    ADS1299_EnableContRead(&afe_driver);
    StartADC();

    // run our own DMA handler
    gpio_add_callback(afe_drdy_spec.port, &afe_drdy_cb_data);
};

void TIBareMetalWrapper::DMAWorkHandler(struct k_work *item) {
    // LOG_INF("TIBareMetalWrapper::%s DRDY signal caught, reading sample", __FUNCTION__);
    ADS1299_ReadOutputSample(&afe_driver);
    PrintCurrentSample();
    DataBufferManager::WriteOneSample(afe_driver.sample);
};

void TIBareMetalWrapper::DMACleanUpHandler(struct k_work *item) {
    gpio_remove_callback(afe_drdy_spec.port, &afe_drdy_cb_data);
    LOG_INF("TIBareMetalWrapper::%s disabling continuous read", __FUNCTION__);

    // Stop adc conversion, then send SDATAC command
    StopADC();
    ADS1299_DisableContRead(&afe_driver);

    // clean up buffer
    DataBufferManager::ResetBuffer();
}

void TIBareMetalWrapper::CheckID() {
    if (is_adc_on) { StopADC(); }

    ADS1299_GetIdState(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s rev id: %u, dev id: %u, num channels: %u",
        __FUNCTION__,
        afe_driver.id.revId, // dont know/care
        afe_driver.id.devId, // should be 3
        afe_driver.id.nuCh   // should be 2 (for 8 channel ADS)
    );
};

void TIBareMetalWrapper::CheckAllRegisters() {
    CheckID();
    CheckConfigRegs();
    CheckChannels();
    CheckLoffSensPState();
    CheckLoffSensNState();
    CheckLoffFlipState();
};

void TIBareMetalWrapper::CheckConfigRegs() {
    if (is_adc_on) { StopADC(); }

    LOG_DBG("reading config 1");
    ADS1299_GetConfig1State(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s config 1 daisyEn: %u, clkEn: %u, dataRate: %u",
        __FUNCTION__,
        afe_driver.config1.daisyEn,
        afe_driver.config1.clkEn,
        afe_driver.config1.dataRate
    );
	k_usleep(1); // wait two clk cycles

    LOG_DBG("reading config 2");
    ADS1299_GetConfig2State(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s config 2 intCal: %u, calAmp: %u, calFreq: %u",
        __FUNCTION__,
        afe_driver.config2.intCal,
        afe_driver.config2.calAmp,
        afe_driver.config2.calFreq
    );	
	k_usleep(1); // wait two clk cycles

    LOG_DBG("reading config 3");
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
	k_usleep(1); // wait two clk cycles
 
    LOG_DBG("reading config 4");
    ADS1299_GetConfig4State(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s config 4 singleShot: %u, pdLoffComp: %u",
        __FUNCTION__,
        afe_driver.config4.singleShot,
        afe_driver.config4.pdLoffComp
    );
	k_usleep(1); // wait two clk cycles

    // update the wrapper's internal flags of current ADC state
};

void TIBareMetalWrapper::CheckChannels(){
    if (is_adc_on) { StopADC(); }
    LOG_DBG("reading chan 1");
    ADS1299_GetCh1SetState(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s channel 1 pd1: %u, gain1: %u, srb2: %u, mux: %u",
        __FUNCTION__,
        afe_driver.ch1set.pd1,
        afe_driver.ch1set.gain1,
        afe_driver.ch1set.srb2,
        afe_driver.ch1set.mux1
    );
	k_usleep(1); // wait two clk cycles
    LOG_DBG("reading chan 2");
    ADS1299_GetCh2SetState(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s channel 2 pd2: %u, gain2: %u, srb2: %u, mux: %u",
        __FUNCTION__,
        afe_driver.ch2set.pd2,
        afe_driver.ch2set.gain2,
        afe_driver.ch2set.srb2,
        afe_driver.ch2set.mux2
    );
	k_usleep(1); // wait two clk cycles
    LOG_DBG("reading chan 3");
    ADS1299_GetCh3SetState(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s channel 3 pd3: %u, gain3: %u, srb2: %u, mux: %u",
        __FUNCTION__,
        afe_driver.ch3set.pd3,
        afe_driver.ch3set.gain3,
        afe_driver.ch3set.srb2,
        afe_driver.ch3set.mux3
    );
	k_usleep(1); // wait two clk cycles
    LOG_DBG("reading chan 4");
    ADS1299_GetCh4SetState(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s channel 4 pd4: %u, gain4: %u, srb2: %u, mux: %u",
        __FUNCTION__,
        afe_driver.ch4set.pd4,
        afe_driver.ch4set.gain4,
        afe_driver.ch4set.srb2,
        afe_driver.ch4set.mux4
    );
	k_usleep(1); // wait two clk cycles
    LOG_DBG("reading chan 5");
    ADS1299_GetCh5SetState(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s channel 5 pd5: %u, gain5: %u, srb2: %u, mux: %u",
        __FUNCTION__,
        afe_driver.ch5set.pd5,
        afe_driver.ch5set.gain5,
        afe_driver.ch5set.srb2,
        afe_driver.ch5set.mux5
    );
	k_usleep(1); // wait two clk cycles
    LOG_DBG("reading chan 6");
    ADS1299_GetCh6SetState(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s channel 6 pd6: %u, gain6: %u, srb2: %u, mux: %u",
        __FUNCTION__,
        afe_driver.ch6set.pd6,
        afe_driver.ch6set.gain6,
        afe_driver.ch6set.srb2,
        afe_driver.ch6set.mux6
    );
	k_usleep(1); // wait two clk cycles
    LOG_DBG("reading chan 7");
    ADS1299_GetCh7SetState(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s channel 7 pd7: %u, gain7: %u, srb2: %u, mux: %u",
        __FUNCTION__,
        afe_driver.ch7set.pd7,
        afe_driver.ch7set.gain7,
        afe_driver.ch7set.srb2,
        afe_driver.ch7set.mux7
    );
	k_usleep(1); // wait two clk cycles
    LOG_DBG("reading chan 8");
    ADS1299_GetCh8SetState(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s channel 8 pd8: %u, gain8: %u, srb2: %u, mux: %u",
        __FUNCTION__,
        afe_driver.ch8set.pd8,
        afe_driver.ch8set.gain8,
        afe_driver.ch8set.srb2,
        afe_driver.ch8set.mux8
    );
	k_usleep(1); // wait two clk cycles
};

void TIBareMetalWrapper::CheckBiasSensPReg() {
    if (is_adc_on) { StopADC(); }
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
    if (is_adc_on) { StopADC(); }
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

void TIBareMetalWrapper::CheckLoffSensPState() {
    if (is_adc_on) { StopADC(); }
    ADS1299_GetLoffSensPState(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s p1: %u, p2: %u, p3: %u, p4: %u, p5: %u, p6: %u, p7: %u, p8: %u",
        __FUNCTION__, 
        afe_driver.loffsensp.loffP1,
        afe_driver.loffsensp.loffP2,
        afe_driver.loffsensp.loffP3,
        afe_driver.loffsensp.loffP4,
        afe_driver.loffsensp.loffP5,
        afe_driver.loffsensp.loffP6,
        afe_driver.loffsensp.loffP7,
        afe_driver.loffsensp.loffP8
    );
};

void TIBareMetalWrapper::CheckLoffSensNState() {
    if (is_adc_on) { StopADC(); }
    ADS1299_GetLoffSensNState(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s n1: %u, n2: %u, n3: %u, n4: %u, n5: %u, n6: %u, n7: %u, n8: %u",
        __FUNCTION__, 
        afe_driver.loffsensn.loffN1,
        afe_driver.loffsensn.loffN2,
        afe_driver.loffsensn.loffN3,
        afe_driver.loffsensn.loffN4,
        afe_driver.loffsensn.loffN5,
        afe_driver.loffsensn.loffN6,
        afe_driver.loffsensn.loffN7,
        afe_driver.loffsensn.loffN8
    );
};

void TIBareMetalWrapper::CheckLoffFlipState(){
    if (is_adc_on) { StopADC(); }
    ADS1299_GetLoffFlipState(&afe_driver);
    LOG_DBG("TIBareMetalWrapper::%s c1: %u, c2: %u, c3: %u, c4: %u, c5: %u, c6: %u, c7: %u, c8: %u",
        __FUNCTION__, 
        afe_driver.loffflip.loffFlip8,
        afe_driver.loffflip.loffFlip7,
        afe_driver.loffflip.loffFlip6,
        afe_driver.loffflip.loffFlip5,
        afe_driver.loffflip.loffFlip4,
        afe_driver.loffflip.loffFlip3,
        afe_driver.loffflip.loffFlip2,
        afe_driver.loffflip.loffFlip1
    );
}

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
}

void TIBareMetalWrapper::TestFakeSampleDataBuffer() {
    // NOTE: this is a blocking loop!
    // clear DMA buffer
    DataBufferManager::Initialize();

    size_t num_fake_samples = sizeof(Utils::inputSignal) / sizeof(Utils::inputSignal[0]);
    sample_t fake_sample = { 0 };

    for(size_t i = 0; i < num_fake_samples; i++) {
        fake_sample.ch1 = Utils::inputSignal[i];
        fake_sample.ch2 = Utils::inputSignal[i];
        fake_sample.ch3 = Utils::inputSignal[i];
        fake_sample.ch4 = Utils::inputSignal[i];
        fake_sample.ch5 = Utils::inputSignal[i];
        fake_sample.ch6 = Utils::inputSignal[i];
        fake_sample.ch7 = Utils::inputSignal[i];
        fake_sample.ch8 = Utils::inputSignal[i];

        DataBufferManager::WriteOneSample(fake_sample);

        // simulate drdy toggle at 250hz
        k_msleep(4);
    }
}

void TIBareMetalWrapper::PrintCurrentSample(){
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

// void TIBareMetalWrapper::HandleDRDYForFullTest(const device *dev, gpio_callback *cb, uint32_t pins){
//     k_work_submit(&dma_test);
// };

void TIBareMetalWrapper::Wakeup() {
};
void TIBareMetalWrapper::Standby() {
};
void TIBareMetalWrapper::Reset() {
};
void TIBareMetalWrapper::ReadData() {
};
