#pragma once

#include <zephyr/sys/util.h>
#include <devicetree_generated.h>
#include "AnalogFrontEndWrapper.h"
#include "DataBufferManager.h"
#include "AFEConfig.h"
#include "drivers/ads1299.h"

class TIBareMetalWrapper : public AnalogFrontEndWrapper {
private:
    static ads1299_t afe_driver;
    static struct gpio_dt_spec afe_reset_spec;
    static struct gpio_dt_spec afe_drdy_spec;
    static struct gpio_dt_spec afe_indicate_spec;
    static struct gpio_callback afe_drdy_cb_data;

    static const struct device* spi_dev;
    static struct spi_config spi_cfg;
    static struct k_poll_signal spi_done_sig;

    static uint8_t rx_buffer[rx_buf_len];
    static bool is_adc_on;
    static bool is_config_reg_continuous;
    static uint8_t master_counter;

    static void DelayMs(uint32_t delay);
    static void DelayUs(uint32_t delay);
    static void Transfer(uint8_t tx[], uint8_t rx[], uint16_t len);
    static void StartDMA(uint8_t rx[], uint16_t len);
    static void SetCS(uint8_t state);
    static void SetReset(uint8_t state);
    static void SetStart(uint8_t state);
    static void SetPWDN(uint8_t state);
    static void HandleDRDY(const device *dev, gpio_callback *cb, uint32_t pins);
    static void ConfigSingleShotConversion();
    static void ConfigContinuousConversion();

public:
    TIBareMetalWrapper();
    ~TIBareMetalWrapper() = default;

    // TODO: consider making these static
    void Initialize() override;
    void Start() override;
    void Wakeup() override;
    void Standby() override;
    void Reset() override;
    void Stop() override;
    void ReadData() override;

    void RunPowerOnTest();    
    void CheckID();
    void CheckChannels();
    void CheckConfigRegs();
    void CheckBiasSensPReg();
    void CheckBiasSensNReg();
    void ReadOneSample();
    void ReadContinuous();
    void TestLoopbackSlave();
};