#include "SigProcThread.h"

LOG_MODULE_REGISTER(eegals_app_core_sig_proc, LOG_LEVEL_DBG);
K_THREAD_STACK_DEFINE(sig_proc_stack_area, SIG_PROC_THREAD_STACK_SIZE_B);
struct k_thread sig_proc_thread_data;

SigProcThread::SigProcThread() {
   
    // NEED TO DO CHECKS
};

void SigProcThread::Initialize() {
    LOG_DBG("SigProc::%s -- initializing Sig Processing", __FUNCTION__);

    if (id == nullptr) {
        LOG_DBG("SigProc::%s -- making thread", __FUNCTION__);
        id = k_thread_create(
            &sig_proc_thread_data, sig_proc_stack_area,
            K_THREAD_STACK_SIZEOF(sig_proc_stack_area),
            SigProcThread::RunThreadSequence,
            this, NULL, NULL,
            SIG_PROC_THREAD_PRIORITY, 0, K_NO_WAIT
        );

        k_thread_name_set(id, "SigProcThread");
        LOG_DBG("SigProc::%s -- thread create successful", __FUNCTION__);
    }
};

void SigProcThread::Run() {
    uint8_t message = 0;
    while (true) {
        if (message_queue.get_with_blocking_wait(message)) {
            uint8_t message_enum = static_cast<SigProcThreadMessage>(message);
		    LOG_DBG("SigProc::%s -- received message: %u at: %u ms", __FUNCTION__, message_enum, k_uptime_get_32());
            switch (message_enum) {
                case COMPUTE_FFT_RESULTS:
                    ComputeSingleSideFFT();
                    break;
                case COMPUTE_POWER_RESULTS:
                    ComputeSingleSidePower();
                    break;
                case COMPUTE_BANDPOWER_RESULTS:
                    ComputeBandPowers(PowerBands::DELTA);
                    break;
                case COMPUTE_RELATIVEPOWER_RESULTS:
                    ComputeRelativeBandPowers();
                    break;
                case INVALID:
                    break;
                default:
                    break;
            }
        }
    }
};

void SigProcThread::TestValuesWooHoo()
{
    printk("\nFilling up allChannels with sample data Woo Hoo\n");
    for (int i = 0; i < RAW_SAMPLE_NUMBER; i++) {
        allChannels.set_at(inputSignal[i], i, 1);
    }
}

void SigProcThread::ComputeSingleSideFFT()
{
    printk("\nPrint single-sided FFT results:\n");
    for (int i = 0; i < 1; i++) 
    {
         // Compute the singeSideFFT for all channels
        channelFFTResults.emplace_back(allChannels.singleSideFFT(i));

        channelFFTResults[i].prettyPrint();
    }
}

void SigProcThread::ComputeSingleSidePower()
{
    printk("\nPrint single-sided power results:\n");
    for (int i = 0; i < CHANNELS; i++) 
    {
        // Compute the singleSidePower for all channels
        channelPowerResults.emplace_back(allChannels.singleSidePower(i));
     
        channelPowerResults[i].prettyPrint();

    }
    
}

void SigProcThread::ComputeBandPowers(const PowerBands powerBand)
{
    uint8_t powerBand_enum = static_cast<PowerBands>(powerBand);
    // Computation of bandpowers
    switch (powerBand_enum) {
        case DELTA:{
            for (int i = 0; i < CHANNELS; i++) {   
                channelBandPowers[i][0] = channelPowerResults[i].singleSideBandPower(i, SAMPLE_FREQ, RAW_SAMPLE_NUMBER, 0);
            }
            break;
        }   
        case THETA:{
            for (int i = 0; i < CHANNELS; i++) {   
                channelBandPowers[i][1] = channelPowerResults[i].singleSideBandPower(i, SAMPLE_FREQ, RAW_SAMPLE_NUMBER, 0);
            }
            break;
        }
        case ALPHA:{
            for (int i = 0; i < CHANNELS; i++) {   
                channelBandPowers[i][2] = channelPowerResults[i].singleSideBandPower(i, SAMPLE_FREQ, RAW_SAMPLE_NUMBER, 0);
            }
            break;
        }
        case BETA:{
            for (int i = 0; i < CHANNELS; i++) {   
                channelBandPowers[i][3] = channelPowerResults[i].singleSideBandPower(i, SAMPLE_FREQ, RAW_SAMPLE_NUMBER, 0);
            }
            break;
        }
        case INVALID:
            break;
        default:
            break;
    }
}

void SigProcThread::ComputeRelativeBandPowers()
{
     for (int i = 0; i < CHANNELS; i++) 
    {
        // Each channel's power spectrum calculates the relative band powers of the 4 bands and stores them into 
        // a 2D array. Outer index denotes the channel number, inner index denotes the band power value
        channelRelativeBandPowers[i] = channelPowerResults[i].singleSideRelativeBandPower(channelBandPowers[i]);
    }
}


