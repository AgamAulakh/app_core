#ifndef ANALOG_FRONT_END_WRAPPER_H
#define ANALOG_FRONT_END_WRAPPER_H

#include "Data.h"

class AnalogFrontEndWrapper {
    
public:
    virtual ~AnalogFrontEndWrapper() = default;
    virtual void Initialize() = 0;
    virtual void Configure() = 0;
    virtual eeg_sample ReadData() = 0;
};

#endif