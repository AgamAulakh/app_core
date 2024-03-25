#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "FFTsignal.h"
#include "ArmMatrixWrapper.h"
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/logging/log.h>
#include "core/Thread.h"
using namespace std;

#define SIG_PROC_THREAD_STACK_SIZE_B 4096
#define SIG_PROC_THREAD_PRIORITY 3 // max based on prj config
#define SIG_PROC_THREAD_MSG_Q_DEPTH 100

#ifndef RAW_SAMPLE_NUMBER
#define RAW_SAMPLE_NUMBER 1024 // Just testing for one channel
#endif
#define PROCESSED_SAMPLE_NUMBER (RAW_SAMPLE_NUMBER/2)
#define SAMPLE_FREQ 250 
#define CHANNELS 1 // Just for testing one channel 
#define BANDS 4

class SigProcThread : public Thread<SIG_PROC_THREAD_MSG_Q_DEPTH> {
private:
    // TIFrontEndWrapper AFEWrapper;
    SigProcThread();
    ~SigProcThread() = default;

    // need to delete copy cstor and assign optor
    SigProcThread(const SigProcThread &) = delete;
    SigProcThread& operator=(const SigProcThread&) = delete;

    // Matrix of raw data
    ArmMatrixWrapper<RAW_SAMPLE_NUMBER, 8> allChannels;

    // Array of FFT results of all 8 channels
    vector< ArmMatrixWrapper<PROCESSED_SAMPLE_NUMBER, 1> > channelFFTResults(CHANNELS);
  
    // Array of Power spectrum of all 8 channels 
    vector< ArmMatrixWrapper<PROCESSED_SAMPLE_NUMBER, 1> > channelPowerResults(CHANNELS);
   
    // Array of channels where each channel has 4 elements for 4 bandpowers
    vector<vector<float32_t>> channelBandPowers(CHANNELS, std::vector<float32_t>(BANDS));

    // Array of channels where each channel has 4 elements for 4 bandpowers
    vector<vector<float32_t>> channelRelativeBandPowers(CHANNELS, std::vector<float32_t>(BANDS));
 

public:
    enum SigProcThreadMessage : uint8_t {
        COMPUTE_FFT_RESULTS = 0,
        COMPUTE_POWER_RESULTS,
        COMPUTE_BANDPOWER_RESULTS,
        COMPUTE_RELATIVEPOWER_RESULTS,
        INVALID,
    };
    enum PowerBands : uint8_t {
        DELTA = 0,
        THETA,
        ALPHA,
        BETA,
        INVALID,
    };

    void Initialize() override;
    void Run() override;
    
    void ComputeSingleSideFFT();
    void ComputeSingleSidePower();
    void ComputeBandPowers();
    void ComputeRelativeBandPowers();

    void TestValuesWooHoo();

    static SigProcThread& GetInstance() {
        // NOTE: this method of using static local variable is thread-safe
        // NOTE: this has to be implemented in the child class
        static SigProcThread instance;
        return instance;
    }
};