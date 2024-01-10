#ifndef CUSTOM_FRONT_END_WRAPPER_H
#define CUSTOM_FRONT_END_WRAPPER_H

#include <zephyr/drivers/spi.h>
#include <zephyr/device.h>

#include "AnalogFrontEndWrapper.h"
#include "DataBufferManager.h"

#define SPI_DEV_NAME DT_LABEL(DT_NODELABEL(spi0))

class CustomFrontEndWrapper : public AnalogFrontEndWrapper {
private:
    static const char *spi_device_name;
    static const struct device *spi_dev;

public:
    CustomFrontEndWrapper();
    ~CustomFrontEndWrapper() = default;
    void Initialize() override;
    void Configure() override;
    eeg_sample ReadData() override;
};

#endif