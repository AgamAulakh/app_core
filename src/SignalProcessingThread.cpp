#include "SignalProcessingThread.h"
#include "Events.h"

#define RAW_SAMPLE_NUMBER 1024
LOG_MODULE_REGISTER(eegals_app_core_sig_proc, LOG_LEVEL_DBG);
K_THREAD_STACK_DEFINE(sig_proc_stack_area, SIG_PROC_THREAD_STACK_SIZE_B);
struct k_thread sig_proc_thread_data;
Semaphore SignalProcessingThread::done_flag = Semaphore();

SignalProcessingThread::SignalProcessingThread() {
    epoch_count = 0;
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
        LOG_INF("SigProc:: -- STARTING TO WAIT FOR NEW MESSAGE");
        if (message_queue.get_with_blocking_wait(message)) {
            uint8_t message_enum = static_cast<SignalProcessingThreadMessage>(message);
		    LOG_DBG("SigProc:: -- received message: %u", message_enum);
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
                    StartProcessing();
                    LOG_INF("SigProc:: -- RETURNING FROM PROCESSING");
                    break;
                case FORCE_STOP_PROCESSING:
                	LOG_INF("SigProc:: cannot force processing (nothing running)");
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
    LOG_DBG("SigProc::%s starting", __FUNCTION__);

    uint8_t message;
    bool is_forced_done = false;
    epoch_count = 0;

    while(epoch_count < max_epochs && !is_forced_done) {
        if(DataBufferManager::GetNumSaplesWritten() >= num_samples_per_epoch) {
            LOG_DBG("SigProc::%s reading epoch", __FUNCTION__);
            DataBufferManager::ReadEpoch(allChannels);
            ComputeSingleSidePower();
            ComputeBandPowers();
            epoch_count++;
            LOG_DBG("SigProc::%s finished epoch %u", __FUNCTION__, epoch_count);
        }
        // check for force stop message
        if (message_queue.peak(message)) {
            uint8_t message_enum = static_cast<SignalProcessingThreadMessage>(message);
		    LOG_DBG("SigProc::%s -- received message: %u", __FUNCTION__, message_enum);
            switch (message_enum) {
                case FORCE_STOP_PROCESSING:
                    is_forced_done = true;
                    // remove the message from the queue instead of just peaking:
                    // message_queue.get(message);
                    break;
                default:
                    LOG_ERR("SigProc::%s cannot respond to this message (process running)", __FUNCTION__);
                    break;
            }
        }
        // sleep until epoch is done
        k_msleep(250);
    }

    LOG_DBG("SigProc::%s stopping %u", __FUNCTION__, epoch_count);

    // construct the final result for output:
    Result to_write = {};
    // to_write.timestamp_ms = k_uptime_get();
    ConvertBandPowerArmMatrixToResult(to_write);
    LCD::SendResult(to_write);

    // stop sigproc
    sig_proc_complete();
    // done processing; return
};

void SignalProcessingThread::TestValuesWooHoo()
{
    printk("\nFilling up allChannels with sample data Woo Hoo\n");
	for (int i = 0; i < RAW_SAMPLE_NUMBER; i++) {
    //    allChannels.set_at(Utils::inputSignal[i], i, 0);
       allChannels.set_at(0.0f, i, 0);
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
    // printk("\nPrint single-sided power results:");
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
                // channelBandPowers.set_at(
                //     channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, DELTA, i), DELTA, i
                // );
				// printk("\nPrint delta power results: %.4f\n", channelBandPowers.at(DELTA,i));
			}
            break;
        }   
        case THETA:{
            for (int i = 0; i < num_electrodes; i++) {   
                // channelBandPowers.set_at(
                //     channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, THETA, i), THETA, i
                // );
				// printk("\nPrint theta power results: %.4f\n", channelBandPowers.at(THETA,i));
            }
            break;
        }
        case ALPHA:{
            for (int i = 0; i < num_electrodes; i++) { 
                // channelBandPowers.set_at(
                //     channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, ALPHA, i), ALPHA, i
                // );  
				// printk("\nPrint alpha power results: %.4f\n", channelBandPowers.at(ALPHA,i));
		    }
            break;
        }
        case BETA:{
            for (int i = 0; i < num_electrodes; i++) {
                // channelBandPowers.set_at(
                //     channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, BETA, i), BETA, i
                // );
				// printk("\nPrint beta power results: %.4f\n", channelBandPowers.at(BETA,i));
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
    // bandpwer is (band) x (epochs) so we add columns
    bandpwer_ch1.set_column_vector_at(ComputeBandPowersPerChannel(CH1_IDX), epoch_count);
    bandpwer_ch2.set_column_vector_at(ComputeBandPowersPerChannel(CH2_IDX), epoch_count);
    bandpwer_ch3.set_column_vector_at(ComputeBandPowersPerChannel(CH3_IDX), epoch_count);
    bandpwer_ch4.set_column_vector_at(ComputeBandPowersPerChannel(CH4_IDX), epoch_count);
    bandpwer_ch5.set_column_vector_at(ComputeBandPowersPerChannel(CH5_IDX), epoch_count);
    bandpwer_ch6.set_column_vector_at(ComputeBandPowersPerChannel(CH6_IDX), epoch_count);
    bandpwer_ch7.set_column_vector_at(ComputeBandPowersPerChannel(CH7_IDX), epoch_count);
    bandpwer_ch8.set_column_vector_at(ComputeBandPowersPerChannel(CH8_IDX), epoch_count);
	//printk("\nPrint band power results:");
    //printk("Lisa loves pretty print\n");
    
    // channelBandPowers.prettyPrint();
};

ArmMatrixWrapper<4,1> SignalProcessingThread::ComputeBandPowersPerChannel(uint32_t electrode) {
    ArmMatrixWrapper<4 , 1> bandPowers;
    bandPowers.set_at(channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, DELTA, electrode), DELTA);
    bandPowers.set_at(channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, THETA, electrode), THETA);
    bandPowers.set_at(channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, ALPHA, electrode), ALPHA);
    bandPowers.set_at(channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, BETA, electrode), BETA);

    // printk("Lisa loves pretty print\n");
    // bandPowers.prettyPrint();
    return bandPowers;
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


void SignalProcessingThread::ConvertBandPowerArmMatrixToResult(Result& to_write)
{
    // dont know how to write this better ahh
    bandpwer_ch1.prettyPrint();
    to_write.band_powers[CH1_IDX].delta = bandpwer_ch1.mean_row(DELTA_IDX);
    to_write.band_powers[CH1_IDX].theta = bandpwer_ch1.mean_row(THETA_IDX);
    to_write.band_powers[CH1_IDX].alpha = bandpwer_ch1.mean_row(ALPHA_IDX);
    to_write.band_powers[CH1_IDX].beta = bandpwer_ch1.mean_row(BETA_IDX);

    // CH2
    bandpwer_ch2.prettyPrint();
    to_write.band_powers[CH2_IDX].delta = bandpwer_ch2.mean_row(DELTA_IDX);
    to_write.band_powers[CH2_IDX].theta = bandpwer_ch2.mean_row(THETA_IDX);
    to_write.band_powers[CH2_IDX].alpha = bandpwer_ch2.mean_row(ALPHA_IDX);
    to_write.band_powers[CH2_IDX].beta = bandpwer_ch2.mean_row(BETA_IDX);

    // CH3
    bandpwer_ch3.prettyPrint();
    to_write.band_powers[CH3_IDX].delta = bandpwer_ch3.mean_row(DELTA_IDX);
    to_write.band_powers[CH3_IDX].theta = bandpwer_ch3.mean_row(THETA_IDX);
    to_write.band_powers[CH3_IDX].alpha = bandpwer_ch3.mean_row(ALPHA_IDX);
    to_write.band_powers[CH3_IDX].beta = bandpwer_ch3.mean_row(BETA_IDX);

    // CH4
    bandpwer_ch4.prettyPrint();
    to_write.band_powers[CH4_IDX].delta = bandpwer_ch4.mean_row(DELTA_IDX);
    to_write.band_powers[CH4_IDX].theta = bandpwer_ch4.mean_row(THETA_IDX);
    to_write.band_powers[CH4_IDX].alpha = bandpwer_ch4.mean_row(ALPHA_IDX);
    to_write.band_powers[CH4_IDX].beta = bandpwer_ch4.mean_row(BETA_IDX);

    // CH5
    bandpwer_ch5.prettyPrint();
    to_write.band_powers[CH5_IDX].delta = bandpwer_ch5.mean_row(DELTA_IDX);
    to_write.band_powers[CH5_IDX].theta = bandpwer_ch5.mean_row(THETA_IDX);
    to_write.band_powers[CH5_IDX].alpha = bandpwer_ch5.mean_row(ALPHA_IDX);
    to_write.band_powers[CH5_IDX].beta = bandpwer_ch5.mean_row(BETA_IDX);

    // CH6
    bandpwer_ch6.prettyPrint();
    to_write.band_powers[CH6_IDX].delta = bandpwer_ch6.mean_row(DELTA_IDX);
    to_write.band_powers[CH6_IDX].theta = bandpwer_ch6.mean_row(THETA_IDX);
    to_write.band_powers[CH6_IDX].alpha = bandpwer_ch6.mean_row(ALPHA_IDX);
    to_write.band_powers[CH6_IDX].beta = bandpwer_ch6.mean_row(BETA_IDX);

    // CH7
    bandpwer_ch7.prettyPrint();
    to_write.band_powers[CH7_IDX].delta = bandpwer_ch7.mean_row(DELTA_IDX);
    to_write.band_powers[CH7_IDX].theta = bandpwer_ch7.mean_row(THETA_IDX);
    to_write.band_powers[CH7_IDX].alpha = bandpwer_ch7.mean_row(ALPHA_IDX);
    to_write.band_powers[CH7_IDX].beta = bandpwer_ch7.mean_row(BETA_IDX);

    // CH8
    bandpwer_ch8.prettyPrint();
    to_write.band_powers[CH8_IDX].delta = bandpwer_ch8.mean_row(DELTA_IDX);
    to_write.band_powers[CH8_IDX].theta = bandpwer_ch8.mean_row(THETA_IDX);
    to_write.band_powers[CH8_IDX].alpha = bandpwer_ch8.mean_row(ALPHA_IDX);
    to_write.band_powers[CH8_IDX].beta = bandpwer_ch8.mean_row(BETA_IDX);
}
