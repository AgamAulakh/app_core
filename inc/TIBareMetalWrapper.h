#pragma once

#include <zephyr/sys/util.h>
#include <devicetree_generated.h>
#include "AnalogFrontEndWrapper.h"
#include "DataBufferManager.h"
#include "AFEConfig.h"
#include "Utils.h"
#include "drivers/ads1299.h"
#include "core/Semaphore.h"

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

    static uint8_t tx_buffer[default_rx_buf_len];
    static uint8_t rx_buffer[default_rx_buf_len];
    static bool is_adc_on;
    static uint8_t master_counter;

    // experimental
    static uint8_t sample_count;
    static struct k_work_q dma_work_queue;

    static void DelayMs(uint32_t delay);
    static void DelayUs(uint32_t delay);
    static void Transfer(uint8_t tx[], uint8_t rx[], uint16_t len);
    static void Read(uint8_t rx[], uint16_t len);
    static void SetCS(uint8_t state);
    static void SetReset(uint8_t state);
    static void SetStart(uint8_t state);
    static void SetPWDN(uint8_t state);
    static void HandleDRDY(const device *dev, gpio_callback *cb, uint32_t pins);

    static void StartADC();
    static void StopADC();
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

    static void RunInputShortTest();    
    static void RunInternalSquareWaveTest();    
    static void CheckID();
    static void CheckChannels();
    static void CheckConfigRegs();
    static void CheckBiasSensPReg();
    static void CheckBiasSensNReg();
    static void ReadOneSample();
    static void ReadContinuous();
    static void PrintCurrentSample();
    static void TestLoopbackSlave();


    // experimental
    static void DMAWorkHandler(struct k_work *item);
    static void DMACleanUpHandler(struct k_work *item);
};