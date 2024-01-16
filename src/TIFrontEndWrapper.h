#pragma once

#include "AnalogFrontEndWrapper.h"
class TIFrontEndWrapper : public AnalogFrontEndWrapper {
public:
    void Initialize() override;
    void Configure() override;
    eeg_sample ReadData() override;
};
