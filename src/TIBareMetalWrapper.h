#pragma once

#include <zephyr/sys/util.h>
#include "AnalogFrontEndWrapper.h"
#include "DataBufferManager.h"
#include "AFEConfig.h"
#include "drivers/ads1299.h"

class TIBareMetalWrapper : public AnalogFrontEndWrapper {
private:
    ads1299_t* afe_driver;
    static void SetCS(uint8_t state);
    static void DelayMs(uint32_t delay);
    static void DelayUs(uint32_t delay);
    static void Transfer(uint8_t tx[], uint8_t rx[], uint16_t len);
    static void SetReset(uint8_t state);
    static void SetStart(uint8_t state);
    static void SetPWDN(uint8_t state);

public:
    TIBareMetalWrapper();
    ~TIBareMetalWrapper() = default;
    void Initialize() override;
    void Start() override;
    void Wakeup() override;
    void Standby() override;
    void Reset() override;
    void Stop() override;
    void ReadData() override;

};