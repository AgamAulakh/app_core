#pragma once

#include "AnalogFrontEndWrapper.h"

class CustomFrontEndWrapper : public AnalogFrontEndWrapper {
public:
    void Initialize() override;
    void Configure();
    void Start() override;
    void Wakeup() override;
    void Standby() override;
    void Reset() override;
    void Stop() override;
    void ReadData() override;
};