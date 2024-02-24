#pragma once

#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>

#include "drivers/ads1299-x.h"
#include "AnalogFrontEndWrapper.h"
#include "DataBufferManager.h"

#define AFE_SPI DT_NODELABEL(afespi)
#define AFE_RESET DT_NODELABEL(led1)
#define AFE_RESET_DEV DT_PHANDLE(AFE_RESET, gpios)
#define AFE_RESET_PIN DT_PHA(AFE_RESET, gpios, pin)
#define AFE_RESET_FLAGS DT_PHA(AFE_RESET, gpios, flags)

// #define AFE_RESET DT_NODELABEL(led1)

// #define AFE_RESET_NODE DT_ALIAS(led1)
// #define AFE_RESET DT_NODELABEL(AFE_RESET_NODE)
// #define AFE_RESET_PIN DT_GPIO_PIN(AFE_RESET_NODE, gpios)

class TIFrontEndWrapper : public AnalogFrontEndWrapper {
private:
    device* afe_spi_device;
    spi_config afe_spi_config;
    ADS1299Driver afe_driver;

public:
    TIFrontEndWrapper();
    ~TIFrontEndWrapper() = default;
    void Initialize() override;
    void Start() override;
    void Wakeup() override;
    void Standby() override;
    void Reset() override;
    void Stop() override;
    void ReadData() override;
};
