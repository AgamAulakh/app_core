#pragma once

#include "Data.h"

class AnalogFrontEndWrapper {
    
public:
    virtual ~AnalogFrontEndWrapper() = default;
    virtual void Initialize() = 0;
    // virtual void Configure() = 0;
    virtual void Start()=0;
    virtual void Wakeup()=0;
    virtual void Standby()=0;
    virtual void Reset()=0;
    virtual void Stop()=0;
    virtual void ReadData()=0;
};