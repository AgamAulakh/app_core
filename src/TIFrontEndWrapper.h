#ifndef TI_FRONT_END_WRAPPER_H
#define TI_FRONT_END_WRAPPER_H

#include "AnalogFrontEndWrapper.h"
class TIFrontEndWrapper : public AnalogFrontEndWrapper {
public:
    void Initialize() override;
    void Configure() override;
    eeg_sample ReadData() override;
};

#endif