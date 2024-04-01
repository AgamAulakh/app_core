#include "SignalProcessingThread.h"
#define RAW_SAMPLE_NUMBER 1024
LOG_MODULE_REGISTER(eegals_app_core_sig_proc, LOG_LEVEL_DBG);
K_THREAD_STACK_DEFINE(sig_proc_stack_area, SIG_PROC_THREAD_STACK_SIZE_B);
struct k_thread sig_proc_thread_data;

SignalProcessingThread::SignalProcessingThread() {
    epoch_count = 0;
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
                    epoch_count = 0;
                    StartProcessing();
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

void SignalProcessingThread::StartProcessing()
{
    // wait until you can read one full epoch
    LOG_DBG("SigProc::%s starting %u ms", __FUNCTION__, k_uptime_get_32());

    while(epoch_count < max_epochs){
        printk("shat");
        if(DataBufferManager::GetNumSaplesWritten() >= num_samples_per_epoch) {
            LOG_DBG("SigProc::%s reading epoch %u ms", __FUNCTION__, k_uptime_get_32());
            DataBufferManager::ReadEpoch(allChannels);
            ComputeSingleSidePower();
            printk("shawt");
            ComputeBandPowers();
            epoch_count++;
            LOG_DBG("SigProc::%s finished epoch %u at %u ms", __FUNCTION__, epoch_count, k_uptime_get_32());
        }
        k_msleep(250);
    }

    printk("pee");
    // stop sigproc
    k_event_post(&s_obj.sig_proc_complete, EVENT_SIG_PROC_COMPLETE);
    // done processing; return
};

void SignalProcessingThread::TestValuesWooHoo()
{
    printk("\nFilling up allChannels with sample data Woo Hoo\n");
	for (int i = 0; i < RAW_SAMPLE_NUMBER; i++) {
       allChannels.set_at(Utils::inputSignal[i], i, 0);
    }
    allChannels.rawFFT(0);
    //allChannels.prettyPrint();
};

void SignalProcessingThread::ComputeSingleSideFFT()
{	
    for (int i = 0; i < num_electrodes; i++) {
        channelFFTResults.set_column_vector_at(allChannels.singleSideFFT(i), i);
    }
    printk("\nPrint single-sided FFT results:\n");
    channelFFTResults.prettyPrint();
};
 

void SignalProcessingThread::ComputeSingleSidePower()
{
    for (int i = 0; i < num_electrodes; i++) 
    {
        channelPowerResults.set_column_vector_at(allChannels.singleSidePower(i), i);
    }
    printk("\nPrint single-sided power results:\n");
    // channelPowerResults.prettyPrint();
};

void SignalProcessingThread::ComputeBandPowerAtOneBand(const PowerBands powerBand)
{
	printk("\nPrint band power results:\n");
    uint8_t powerBand_enum = static_cast<PowerBands>(powerBand);
    // Computation of bandpowers
    switch (powerBand_enum) {
        case DELTA:{
            for (int i = 0; i < num_electrodes; i++) {   
                channelBandPowers.set_at(
                    channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, DELTA), DELTA, i
                );
				printk("\nPrint delta power results: %.4f\n", channelBandPowers.at(DELTA,i));
			}
            break;
        }   
        case THETA:{
            for (int i = 0; i < num_electrodes; i++) {   
                channelBandPowers.set_at(
                    channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, THETA), THETA, i
                );
				printk("\nPrint theta power results: %.4f\n", channelBandPowers.at(THETA,i));
            }
            break;
        }
        case ALPHA:{
            for (int i = 0; i < num_electrodes; i++) { 
                channelBandPowers.set_at(
                    channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, ALPHA), ALPHA, i
                );  
				printk("\nPrint alpha power results: %.4f\n", channelBandPowers.at(ALPHA,i));
		    }
            break;
        }
        case BETA:{
            for (int i = 0; i < num_electrodes; i++) {
                channelBandPowers.set_at(
                    channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, BETA), BETA, i
                );
				printk("\nPrint beta power results: %.4f\n", channelBandPowers.at(BETA,i));
			}
            break;
        }
        case INVALID:
            break;
        default:
            break;
    }
};

void SignalProcessingThread::ComputeBandPowers() {
    for (int i = 0; i < num_electrodes; i++) {   
        channelBandPowers.set_at(
            channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, DELTA), DELTA, i
        );
        channelBandPowers.set_at(
            channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, THETA), THETA, i
        );
        channelBandPowers.set_at(
            channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, ALPHA), ALPHA, i
        );
        channelBandPowers.set_at(
            channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, BETA), BETA, i
        );
        printk("imagine dragons");
    }
	printk("\nPrint band power results:\n");
    channelBandPowers.prettyPrint();
};

void SignalProcessingThread::ComputeRelativeBandPowers()
{
    for (int i = 0; i < num_electrodes; i++) 
    {
        // Each channel's power spectrum calculates the relative band powers of the 4 bands and stores them into 
        // a 2D array. Outer index denotes the channel number, inner index denotes the band power value
        // channelRelativeBandPowers[i] = channelPowerResults[i].singleSideRelativeBandPower(channelBandPowers[i]);
    }
};


