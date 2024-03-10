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
    static struct gpio_dt_spec afe_indicate_spec;

    static const struct device* spi_dev;
    static struct spi_config spi_cfg;
    static struct k_poll_signal spi_done_sig;

    static void DelayMs(uint32_t delay);
    static void DelayUs(uint32_t delay);
    static void Transfer(uint8_t tx[], uint8_t rx[], uint16_t len);
    static void SetCS(uint8_t state);
    static void SetReset(uint8_t state);
    static void SetStart(uint8_t state);
    static void SetPWDN(uint8_t state);

    static uint8_t master_counter;

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

    void TestLoopbackSlave();
};