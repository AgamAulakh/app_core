#pragma once
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "ArmMatrixWrapper.h"
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/logging/log.h>
#include "core/Thread.h"
#include "core/Semaphore.h"
#include "DataBufferManager.h"
#include "Data.h"
#include "Utils.h"

#define SIG_PROC_THREAD_STACK_SIZE_B 32768
#define SIG_PROC_THREAD_PRIORITY 4 // max based on prj config
#define SIG_PROC_THREAD_MSG_Q_DEPTH 10

// #ifndef RAW_SAMPLE_NUMBER
// #define RAW_SAMPLE_NUMBER 1024 // Just testing for one channel
// #endif
/// @brief 
class SignalProcessingThread : public Thread<SIG_PROC_THREAD_MSG_Q_DEPTH> {
private:
    // TIFrontEndWrapper AFEWrapper;
    SignalProcessingThread();
    ~SignalProcessingThread() = default;

    // need to delete copy cstor and assign optor
    SignalProcessingThread(const SignalProcessingThread &) = delete;
    SignalProcessingThread& operator=(const SignalProcessingThread&) = delete;


    // heehee variables
    uint8_t epoch_count;

    // Matrix of raw data
    ArmMatrixWrapper<num_samples_per_epoch, num_electrodes> allChannels;

    // Array of FFT results of all 8 channels
    ArmMatrixWrapper<num_samples_per_epoch/2, num_electrodes> channelFFTResults;
  
    // Array of Power spectrum of all 8 channels 
    ArmMatrixWrapper<num_samples_per_epoch/2, num_electrodes> channelPowerResults;
   
    //vector<float32_t> bandPowers = vector<float32_t>(4);
    // Array of channels where each channel has 4 elements for 4 bandpowers
    ArmMatrixWrapper<4, max_epochs> bandpwer_ch1;
    ArmMatrixWrapper<4, max_epochs> bandpwer_ch2;
    ArmMatrixWrapper<4, max_epochs> bandpwer_ch3;
    ArmMatrixWrapper<4, max_epochs> bandpwer_ch4;
    ArmMatrixWrapper<4, max_epochs> bandpwer_ch5;
    ArmMatrixWrapper<4, max_epochs> bandpwer_ch6;
    ArmMatrixWrapper<4, max_epochs> bandpwer_ch7;
    ArmMatrixWrapper<4, max_epochs> bandpwer_ch8;
    
    //vector<vector<float32_t>> channelBandPowers = vector<vector<float32_t>>(1, bandPowers);
    ArmMatrixWrapper<4, num_electrodes> channelRelativeBandPowers;
    // Array of channels where each channel has 4 elements for 4 bandpowers
    //vector<vector<float32_t>> channelRelativeBandPowers = vector<vector<float32_t> >(1, bandPowers);
 

public:
    static Semaphore done_flag;
    enum SignalProcessingThreadMessage : uint8_t {
        COMPUTE_DEBUG_FFT_RESULTS = 0,
        COMPUTE_DEBUG_POWER_RESULTS,
        COMPUTE_DEBUG_BANDPOWER_RESULTS,
        COMPUTE_DEBUG_RELATIVEPOWER_RESULTS,
        START_PROCESSING,
        FORCE_STOP_PROCESSING,
        INVALID,
    };
    enum PowerBands : uint8_t {
        DELTA = 0,
        THETA,
        ALPHA,
        BETA,
        NONE,
    };

    void Initialize() override;
    void Run() override;
    void StartProcessing();

    void ComputeSingleSideFFT();
    void ComputeSingleSidePower();
    void ComputeBandPowerAtOneBand(const PowerBands powerBand);
    ArmMatrixWrapper<4,1> ComputeBandPowersPerChannel(uint32_t electrode);
    void ComputeBandPowers();
    void ComputeRelativeBandPowers();

    void TestValuesWooHoo();

    static SignalProcessingThread& GetInstance() {
        // NOTE: this method of using static local variable is thread-safe
        // NOTE: this has to be implemented in the child class
        static SignalProcessingThread instance;
        return instance;
    }
};