#include "SignalProcessingThread.h"
#define RAW_SAMPLE_NUMBER 512
LOG_MODULE_REGISTER(eegals_app_core_sig_proc, LOG_LEVEL_DBG);
K_THREAD_STACK_DEFINE(sig_proc_stack_area, SIG_PROC_THREAD_STACK_SIZE_B);
struct k_thread sig_proc_thread_data;
Semaphore SignalProcessingThread::done_flag = Semaphore();

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
					PopulateTestValues();
                    ComputeSingleSideFFT();
                    break;
                case COMPUTE_DEBUG_POWER_RESULTS:
					PopulateTestValues();
                    ComputeSingleSidePower();
                    break;
                case COMPUTE_DEBUG_BANDPOWER_RESULTS:
					PopulateTestValues();
                    ComputeSingleSidePower();
                    ComputeBandPowers();
                    ComputeAverageBandPowers();
                    ComputeRelativeBandPowers();
                    RelativeBandPowerVariance();
                    AverageBandPowerVariance();
                    varianceSummation();
                    Classification();
                    break;
                case COMPUTE_DEBUG_RELATIVEPOWER_RESULTS:
                    ComputeRelativeBandPowers();
                    break;
                case START_PROCESSING:
                    epoch_count = 0;
                    done_flag.wait();
                    StartProcessing();
                    done_flag.give();
                    // to be set by state machine
                    break;
                case FORCE_STOP_PROCESSING:
                	LOG_ERR("SigProc::%s cannot force processing (nothing running)", __FUNCTION__);
                    break;
                case DEBUG_CLASSIFICATION:
                    Classification();
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
    uint8_t message;
    bool is_forced_done = false;
    while(epoch_count < max_epochs || !is_forced_done){
        if(DataBufferManager::GetNumSaplesWritten() >= num_samples_per_epoch) {
            LOG_DBG("SigProc::%s reading epoch %u ms", __FUNCTION__, k_uptime_get_32());
            DataBufferManager::ReadEpoch(allChannels);
            ComputeSingleSidePower();
            ComputeBandPowers();
            epoch_count++;
            LOG_DBG("SigProc::%s finished epoch %u at %u ms", __FUNCTION__, epoch_count, k_uptime_get_32());
        }
        // check for force stop message
        if (message_queue.peak(message)) {
            uint8_t message_enum = static_cast<SignalProcessingThreadMessage>(message);
		    LOG_DBG("SigProc::%s -- received message: %u at: %u ms", __FUNCTION__, message_enum, k_uptime_get_32());
            switch (message_enum) {
                case FORCE_STOP_PROCESSING:
                    is_forced_done = true;
                    break;
                default:
                    LOG_ERR("SigProc::%s cannot respond to this message (process running)", __FUNCTION__);
                    break;
            }
        }
        // sleep until epoch is done
        k_msleep(250);
    }

    LOG_DBG("SigProc::%s stopping %u at %u ms", __FUNCTION__, epoch_count, k_uptime_get_32());

    // stop sigproc
    // k_event_post(&s_obj.sig_proc_complete, EVENT_SIG_PROC_COMPLETE);
    // done processing; return
};

void SignalProcessingThread::PopulateTestValues()
{
    printk("\nFilling up all Channels with Square Wave values\n");
    // for (int i = 0; i < num_electrodes; i++) {

    //     for (int j = 0; j < RAW_SAMPLE_NUMBER; j++) {

    //         allChannels.set_at(Utils::inputSignal[j], j, i);
    //     }
    // }
    // Populate for the square wave test 
	for (int i = 0; i < RAW_SAMPLE_NUMBER; i++) {
        allChannels.set_at(Utils::channelOne[i], i, 0);
        allChannels.set_at(Utils::channelTwo[i], i, 1);
        allChannels.set_at(Utils::channelThree[i], i, 2);
        allChannels.set_at(Utils::channelFour[i], i, 3);
        allChannels.set_at(Utils::channelFive[i], i, 4);
        allChannels.set_at(Utils::channelSix[i], i, 5);
        allChannels.set_at(Utils::channelSeven[i], i, 6);
        allChannels.set_at(Utils::channelEight[i], i, 7);
    }
   //allChannels.prettyPrint();
};

void SignalProcessingThread::ComputeSingleSideFFT()
{	
    for (int i = 0; i < num_electrodes; i++) {
        channelFFTResults.set_column_vector_at(allChannels.singleSideFFT(i), i);
    }
   // printk("\nPrint single-sided FFT results:\n");
    ///channelFFTResults.prettyPrint();
};
 

void SignalProcessingThread::ComputeSingleSidePower()
{
    for (int i = 0; i < num_electrodes; i++) 
    {
        channelPowerResults.set_column_vector_at(allChannels.singleSidePower(i), i);
    }
};

void SignalProcessingThread::ComputeBandPowers() {
    // bandpwer is (band) x (epochs) so we add columns
     for (int epoch_i = 0; epoch_i < 8; epoch_i++) 
    {
        bandpwer_ch1.set_column_vector_at(ComputeBandPowersPerChannel(CH1_IDX), epoch_i);
        bandpwer_ch2.set_column_vector_at(ComputeBandPowersPerChannel(CH2_IDX), epoch_i);
        bandpwer_ch3.set_column_vector_at(ComputeBandPowersPerChannel(CH3_IDX), epoch_i);
        bandpwer_ch4.set_column_vector_at(ComputeBandPowersPerChannel(CH4_IDX), epoch_i);
        bandpwer_ch5.set_column_vector_at(ComputeBandPowersPerChannel(CH5_IDX), epoch_i);
        bandpwer_ch6.set_column_vector_at(ComputeBandPowersPerChannel(CH6_IDX), epoch_i);
        bandpwer_ch7.set_column_vector_at(ComputeBandPowersPerChannel(CH7_IDX), epoch_i);
        bandpwer_ch8.set_column_vector_at(ComputeBandPowersPerChannel(CH8_IDX), epoch_i);
     }
};

ArmMatrixWrapper<4,1> SignalProcessingThread::ComputeBandPowersPerChannel(uint32_t electrode) {
    ArmMatrixWrapper<4 , 1> bandPowers;
    bandPowers.set_at(channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, DELTA, electrode), DELTA);
    bandPowers.set_at(channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, THETA, electrode), THETA);
    bandPowers.set_at(channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, ALPHA, electrode), ALPHA);
    bandPowers.set_at(channelPowerResults.singleSideBandPower(SAMPLE_FREQ, RAW_SAMPLE_NUMBER, BETA, electrode), BETA);
    //printk("YOLO");
   // bandPowers.prettyPrint();
    return bandPowers;
};

void SignalProcessingThread::ComputeAverageBandPowers() 
{
    for (int band = 0; band < BANDS; band++) 
    {
        averageBandPowers.set_at(bandpwer_ch1.get_row_vector_at(band).mean(), band, 0);
        averageBandPowers.set_at(bandpwer_ch2.get_row_vector_at(band).mean(), band, 1);
        averageBandPowers.set_at(bandpwer_ch3.get_row_vector_at(band).mean(), band, 2);
        averageBandPowers.set_at(bandpwer_ch4.get_row_vector_at(band).mean(), band, 3);
        averageBandPowers.set_at(bandpwer_ch5.get_row_vector_at(band).mean(), band, 4);
        averageBandPowers.set_at(bandpwer_ch6.get_row_vector_at(band).mean(), band, 5);
        averageBandPowers.set_at(bandpwer_ch7.get_row_vector_at(band).mean(), band, 6);
        averageBandPowers.set_at(bandpwer_ch8.get_row_vector_at(band).mean(), band, 7);
           
    }
    //printk("\nAverage Band Power Results:\n");
    //averageBandPowers.prettyPrint();
}

void SignalProcessingThread::ComputeRelativeBandPowers(){

    for (int i = 0; i < num_electrodes; i++) {
        relativeBandPowers.set_column_vector_at(channelPowerResults.singleSideRelativeBandPower(averageBandPowers.get_column_vector_at(i), i), i);
    }

    relativeBandPowers.prettyPrint();
}

void SignalProcessingThread::AverageBandPowerVariance()
{
    // Set the last electrode to nothing
    averageBandPowers.set_at(0, DELTA, 7);
    averageBandPowers.set_at(0, THETA, 7);
    averageBandPowers.set_at(0, ALPHA, 7);
    averageBandPowers.set_at(0, BETA, 7);
    
    // Matrix is 4 x 8 but set to one that is 4 x 1
    averageBandPowersVariance.set_at(averageBandPowers.get_row_vector_at(DELTA).variance(), DELTA, 0);
    averageBandPowersVariance.set_at(averageBandPowers.get_row_vector_at(THETA).variance(), THETA, 0);
    averageBandPowersVariance.set_at(averageBandPowers.get_row_vector_at(ALPHA).variance(), ALPHA, 0);
    averageBandPowersVariance.set_at(averageBandPowers.get_row_vector_at(BETA).variance(), BETA, 0);
    
    printk("\nAVERAGE BAND POWER VARIANCE:\n");
    averageBandPowersVariance.prettyPrint();
   // }
}

void SignalProcessingThread::RelativeBandPowerVariance()
{
    // Set the last electrode to nothing
    relativeBandPowers.set_at(0, DELTA, 7);
    relativeBandPowers.set_at(0, THETA, 7);
    relativeBandPowers.set_at(0, ALPHA, 7);
    relativeBandPowers.set_at(0, BETA, 7);

    // Matrix is 4 x 8 but set to one that is 4 x 1
    relativeBandPowersVariance.set_at(relativeBandPowers.get_row_vector_at(DELTA).variance(), DELTA, 0);
    relativeBandPowersVariance.set_at(relativeBandPowers.get_row_vector_at(THETA).variance(), THETA, 0);
    relativeBandPowersVariance.set_at(relativeBandPowers.get_row_vector_at(ALPHA).variance(), ALPHA, 0);
    relativeBandPowersVariance.set_at(relativeBandPowers.get_row_vector_at(BETA).variance(), BETA, 0);
    
    printk("\nRELATIVE BAND POWER VARIANCE:\n");
    relativeBandPowersVariance.prettyPrint();
    
}

void SignalProcessingThread::varianceSummation()
{
    float32_t averageBandPowerVarianceSum = 0;
    float32_t relativeBandPowerVarianceSum = 0;

    // Summation of the variance across all brain waves
    averageBandPowerVarianceSum += averageBandPowersVariance.at(DELTA,0);
    averageBandPowerVarianceSum += averageBandPowersVariance.at(THETA,0);
    averageBandPowerVarianceSum += averageBandPowersVariance.at(ALPHA,0);
    averageBandPowerVarianceSum += averageBandPowersVariance.at(BETA,0);

    relativeBandPowerVarianceSum += relativeBandPowersVariance.at(DELTA,0);
    relativeBandPowerVarianceSum += relativeBandPowersVariance.at(THETA,0);
    relativeBandPowerVarianceSum += relativeBandPowersVariance.at(ALPHA,0);
    relativeBandPowerVarianceSum += relativeBandPowersVariance.at(BETA,0);

   
    printk("\nLISA PRINT AVERAGE BAND POWER VARIANCE SUM %.10f:\n",  averageBandPowerVarianceSum);
    printk("\nLISA PRINT RELATIVE BAND POWER VARIANCE SUM %.10f:\n",  relativeBandPowerVarianceSum);

    classificationVariance = averageBandPowerVarianceSum;
}

bool SignalProcessingThread::Classification(){

// Based on the variance values of the bands for one dropped electrode, 
// if sum is more than the order of magnitude of 1E-5 then it is abnormal
    bool concussion = false;
    if (0.0039911643 <= classificationVariance)
    {
        concussion = true;
    }

    if(concussion)
    {
        printk("\nYou have a concussion\n");
    }
    else 
    {
        printk("\nYou DO NOT have a concussion\n");
    }

    return concussion;

}

// bool SignalProcessingThread::Classification(){

//     // For Square Wave Test Config2: 01
//     // Frequency = 1.9531 Hz
//     // Expected Amplitude = 1.875 mV
//     bool concussion = false;
//     // Four if statements for classification for the four bands (Delta, Theta, Alpha, Beta)
//     // for the Square Wave. We should have a standard of give or take +-0.0005
//     float32_t deviation = 0.0005;
    
//     float32_t standardDelta = 0.1773;
//     float32_t lowDelta = standardDelta - deviation;
//     float32_t highDelta = standardDelta + deviation;
    
//     float32_t standardTheta = 0.0196;
//     float32_t lowTheta = standardTheta - deviation;
//     float32_t highTheta = standardTheta + deviation;

//     float32_t standardAlpha = 0.0070;
//     float32_t lowAlpha = standardAlpha - deviation;
//     float32_t highAlpha = standardAlpha + deviation;

//     float32_t standardBeta = 0.0086;
//     float32_t lowBeta = standardBeta - deviation;
//     float32_t highBeta = standardBeta + deviation;

//     for (int electrode = 0; electrode < num_electrodes; electrode++) 
//     {
        
//         if (averageBandPowers.at(DELTA, electrode) > highDelta || averageBandPowers.at(DELTA, electrode) < lowDelta){
//             concussion = true;
//             break;
//         }

//         if (averageBandPowers.at(THETA, electrode) > highTheta || averageBandPowers.at(THETA, electrode) < lowTheta){
//             concussion = true;
//             break;
//         }

//         if (averageBandPowers.at(ALPHA, electrode) > highAlpha || averageBandPowers.at(ALPHA, electrode) < lowAlpha){
//             concussion = true;
//             break;
//         }

//         if (averageBandPowers.at(BETA, electrode) > highBeta || averageBandPowers.at(BETA, electrode) < lowBeta){
//             concussion = true;
//             break;
//         }
    
//     }

//     if(concussion)
//     {
//         printk("\nYou have a concussion\n");
//     }
//     else 
//     {
//         printk("\nYou DO NOT have a concussion\n");
//     }
    
//     return concussion;
// }


