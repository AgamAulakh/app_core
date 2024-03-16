#include "SignalProcessingThread.h"
#define RAW_SAMPLE_NUMBER 1024
LOG_MODULE_REGISTER(eegals_app_core_sig_proc, LOG_LEVEL_DBG);
K_THREAD_STACK_DEFINE(sig_proc_stack_area, SIG_PROC_THREAD_STACK_SIZE_B);
struct k_thread sig_proc_thread_data;

// float32_t inputSignal[16] = {10.0,20.0,
// 							10.0,20.0,
// 							10.0,20.0,
// 							10.0,20.0,
// 							10.0,20.0,
// 							10.0,20.0,
// 							10.0,20.0,
// 							10.0,20.0};

SignalProcessingThread::SignalProcessingThread() {
   
    // NEED TO DO CHECKS
};

void SignalProcessingThread::Initialize() {
    LOG_DBG("SigProc::%s -- initializing Sig Processing", __FUNCTION__);

    if (id == nullptr) {
        LOG_DBG("SigProc::%s -- making thread", __FUNCTION__);
        id = k_thread_create(
            &sig_proc_thread_data, sig_proc_stack_area,
            K_THREAD_STACK_SIZEOF(sig_proc_stack_area),
            SignalProcessingThread::RunThreadSequence,
            this, NULL, NULL,
            SIG_PROC_THREAD_PRIORITY, 0, K_NO_WAIT
        );

        k_thread_name_set(id, "SignalProcessingThread");
        LOG_DBG("SigProc::%s -- thread create successful", __FUNCTION__);
    }
};

void SignalProcessingThread::Run() {
    uint8_t message = 0;
    while (true) {
        if (message_queue.get_with_blocking_wait(message)) {
            uint8_t message_enum = static_cast<SignalProcessingThreadMessage>(message);
		    LOG_DBG("SigProc::%s -- received message: %u at: %u ms", __FUNCTION__, message_enum, k_uptime_get_32());
            switch (message_enum) {
                case COMPUTE_DEBUG_FFT_RESULTS:
					TestValuesWooHoo();
                   // ComputeSingleSideFFT();
                    break;
                case COMPUTE_DEBUG_POWER_RESULTS:
					TestValuesWooHoo();
                    ComputeSingleSidePower();
                    break;
                case COMPUTE_DEBUG_BANDPOWER_RESULTS:
					TestValuesWooHoo();
                    ComputeSingleSidePower();
                    //ComputeBandPowers(PowerBands::DELTA);
                    break;
                case COMPUTE_DEBUG_RELATIVEPOWER_RESULTS:
                    ComputeRelativeBandPowers();
                    break;
                case START_PROCESSING:
                    // to be set by state machine
                    break;
                case INVALID:
                    break;
                default:
                    break;
            }
        }
    }
};

void SignalProcessingThread::TestValuesWooHoo()
{
    printk("\nFilling up allChannels with sample data Woo Hoo\n");
	for (int i = 0; i < RAW_SAMPLE_NUMBER; i++) {
       allChannels.set_at(inputSignal[i], i, 0);
    }
    allChannels.rawFFT(0);


    //allChannels.prettyPrint();

}

void SignalProcessingThread::ComputeSingleSideFFT()
{
	
    printk("\nPrint single-sided FFT results:\n");
	
	for (int i = 0; i < CHANNELS; i++) {
        //Compute the singeSideFFT for all channels
		channelFFTResults.emplace_back(allChannels.singleSideFFT(i));
        channelFFTResults[i].prettyPrint();
    }
}
 

void SignalProcessingThread::ComputeSingleSidePower()
{
    printk("\nPrint single-sided power results:\n");
    for (int i = 0; i < CHANNELS; i++) 
    {
        // Compute the singleSidePower for all channels
        channelPowerResults.emplace_back(allChannels.singleSidePower(i));
        channelPowerResults[i].prettyPrint();

    }
    
}

void SignalProcessingThread::ComputeBandPowers(const PowerBands powerBand)
{
	printk("\nPrint band power results:\n");
    uint8_t powerBand_enum = static_cast<PowerBands>(powerBand);
    // Computation of bandpowers
    switch (powerBand_enum) {
        case DELTA:{
            for (int i = 0; i < CHANNELS; i++) {   
				ArmMatrixWrapper<512, 1> channelBandPowerResult;
				 
                channelBandPowers[i][0] = channelBandPowerResult.singleSideBandPower(i, SAMPLE_FREQ, RAW_SAMPLE_NUMBER, 0);
				printk("\nPrint delta power results: %.4f\n", channelBandPowers[i][0]);
			}
            break;
        }   
        case THETA:{
            for (int i = 0; i < CHANNELS; i++) {   
                channelBandPowers[i][1] = channelPowerResults[i].singleSideBandPower(i, SAMPLE_FREQ, RAW_SAMPLE_NUMBER, 0);
				printk("\nPrint theta power results: %.4f\n", channelBandPowers[i][1]);
            }
            break;
        }
        case ALPHA:{
            for (int i = 0; i < CHANNELS; i++) {   
                channelBandPowers[i][2] = channelPowerResults[i].singleSideBandPower(i, SAMPLE_FREQ, RAW_SAMPLE_NUMBER, 0);
				printk("\nPrint alpha power results: %.4f\n", channelBandPowers[i][2]);
		    }
            break;
        }
        case BETA:{
            for (int i = 0; i < CHANNELS; i++) {   
                channelBandPowers[i][3] = channelPowerResults[i].singleSideBandPower(i, SAMPLE_FREQ, RAW_SAMPLE_NUMBER, 0);
				printk("\nPrint beta power results: %.4f\n", channelBandPowers[i][3]);
			}
            break;
        }
        case INVALID:
            break;
        default:
            break;
    }
}

void SignalProcessingThread::ComputeRelativeBandPowers()
{
     for (int i = 0; i < CHANNELS; i++) 
    {
        // Each channel's power spectrum calculates the relative band powers of the 4 bands and stores them into 
        // a 2D array. Outer index denotes the channel number, inner index denotes the band power value
        channelRelativeBandPowers[i] = channelPowerResults[i].singleSideRelativeBandPower(channelBandPowers[i]);
    }
}


