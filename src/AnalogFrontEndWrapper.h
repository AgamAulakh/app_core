#pragma once

#include "Data.h"

class AnalogFrontEndWrapper {
    
public:
    virtual ~AnalogFrontEndWrapper() = default;
    virtual void Initialize() = 0;
    virtual void Configure() = 0;
    virtual eeg_sample ReadData() = 0;
};