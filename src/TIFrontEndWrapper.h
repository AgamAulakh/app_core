#pragma once

#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>

#include "drivers/ads1299-x.h"
#include "AnalogFrontEndWrapper.h"
#include "DataBufferManager.h"

#define AFE_SPI DT_NODELABEL(spi_afe)

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
