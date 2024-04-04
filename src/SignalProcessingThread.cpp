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

    // need to get average band powers for relative bps
    ComputeAverageBandPowers();
    ComputeRelativeBandPowers();
    // only need to calculate relative bpVar
    RelativeBandPowerVariance();
    varianceSummation();

    // construct the final result for output:
    Result to_write = {};

    // write the concussion reuslt (true = abnormal)
    to_write.is_abnormal = Classification();
    ConvertBandPowerArmMatrixToResult(to_write);

    // update LCD screen
    LCD::update_most_recent_result(to_write);

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

    classificationVariance = relativeBandPowerVarianceSum;
}

bool SignalProcessingThread::Classification(){

// Based on the variance values of the bands for one dropped electrode, 
// if sum is more than the order of magnitude of 1E-5 then it is abnormal
    bool concussion = false;
    if (0.0039911643f <= classificationVariance)
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

void SignalProcessingThread::ConvertBandPowerArmMatrixToResult(Result& to_write)
{
    // dont know how to write this better ahh
    bandpwer_ch1.prettyPrint();
    to_write.band_powers[CH1_IDX].delta = bandpwer_ch1.mean_row(DELTA_IDX);
    to_write.band_powers[CH1_IDX].theta = bandpwer_ch1.mean_row(THETA_IDX);
    to_write.band_powers[CH1_IDX].alpha = bandpwer_ch1.mean_row(ALPHA_IDX);
    to_write.band_powers[CH1_IDX].beta = bandpwer_ch1.mean_row(BETA_IDX);
    LOG_INF("delta: %f, theta: %f, alpha: %f, beta: %f",
        to_write.band_powers[CH1_IDX].delta,
        to_write.band_powers[CH1_IDX].theta,
        to_write.band_powers[CH1_IDX].alpha,
        to_write.band_powers[CH1_IDX].beta
    );

    // CH2
    bandpwer_ch2.prettyPrint();
    to_write.band_powers[CH2_IDX].delta = bandpwer_ch2.mean_row(DELTA_IDX);
    to_write.band_powers[CH2_IDX].theta = bandpwer_ch2.mean_row(THETA_IDX);
    to_write.band_powers[CH2_IDX].alpha = bandpwer_ch2.mean_row(ALPHA_IDX);
    to_write.band_powers[CH2_IDX].beta = bandpwer_ch2.mean_row(BETA_IDX);
    LOG_INF("delta: %f, theta: %f, alpha: %f, beta: %f",
        to_write.band_powers[CH2_IDX].delta,
        to_write.band_powers[CH2_IDX].theta,
        to_write.band_powers[CH2_IDX].alpha,
        to_write.band_powers[CH2_IDX].beta
    );

    // CH3
    bandpwer_ch3.prettyPrint();
    to_write.band_powers[CH3_IDX].delta = bandpwer_ch3.mean_row(DELTA_IDX);
    to_write.band_powers[CH3_IDX].theta = bandpwer_ch3.mean_row(THETA_IDX);
    to_write.band_powers[CH3_IDX].alpha = bandpwer_ch3.mean_row(ALPHA_IDX);
    to_write.band_powers[CH3_IDX].beta = bandpwer_ch3.mean_row(BETA_IDX);
    LOG_INF("delta: %f, theta: %f, alpha: %f, beta: %f",
        to_write.band_powers[CH3_IDX].delta,
        to_write.band_powers[CH3_IDX].theta,
        to_write.band_powers[CH3_IDX].alpha,
        to_write.band_powers[CH3_IDX].beta
    );

    // CH4
    bandpwer_ch4.prettyPrint();
    to_write.band_powers[CH4_IDX].delta = bandpwer_ch4.mean_row(DELTA_IDX);
    to_write.band_powers[CH4_IDX].theta = bandpwer_ch4.mean_row(THETA_IDX);
    to_write.band_powers[CH4_IDX].alpha = bandpwer_ch4.mean_row(ALPHA_IDX);
    to_write.band_powers[CH4_IDX].beta = bandpwer_ch4.mean_row(BETA_IDX);
    LOG_INF("delta: %f, theta: %f, alpha: %f, beta: %f",
        to_write.band_powers[CH4_IDX].delta,
        to_write.band_powers[CH4_IDX].theta,
        to_write.band_powers[CH4_IDX].alpha,
        to_write.band_powers[CH4_IDX].beta
    );

    // CH5
    bandpwer_ch5.prettyPrint();
    to_write.band_powers[CH5_IDX].delta = bandpwer_ch5.mean_row(DELTA_IDX);
    to_write.band_powers[CH5_IDX].theta = bandpwer_ch5.mean_row(THETA_IDX);
    to_write.band_powers[CH5_IDX].alpha = bandpwer_ch5.mean_row(ALPHA_IDX);
    to_write.band_powers[CH5_IDX].beta = bandpwer_ch5.mean_row(BETA_IDX);
    LOG_INF("delta: %f, theta: %f, alpha: %f, beta: %f",
        to_write.band_powers[CH5_IDX].delta,
        to_write.band_powers[CH5_IDX].theta,
        to_write.band_powers[CH5_IDX].alpha,
        to_write.band_powers[CH5_IDX].beta
    );

    // CH6
    bandpwer_ch6.prettyPrint();
    to_write.band_powers[CH6_IDX].delta = bandpwer_ch6.mean_row(DELTA_IDX);
    to_write.band_powers[CH6_IDX].theta = bandpwer_ch6.mean_row(THETA_IDX);
    to_write.band_powers[CH6_IDX].alpha = bandpwer_ch6.mean_row(ALPHA_IDX);
    to_write.band_powers[CH6_IDX].beta = bandpwer_ch6.mean_row(BETA_IDX);
    LOG_INF("delta: %f, theta: %f, alpha: %f, beta: %f",
        to_write.band_powers[CH6_IDX].delta,
        to_write.band_powers[CH6_IDX].theta,
        to_write.band_powers[CH6_IDX].alpha,
        to_write.band_powers[CH6_IDX].beta
    );

    // CH7
    bandpwer_ch7.prettyPrint();
    to_write.band_powers[CH7_IDX].delta = bandpwer_ch7.mean_row(DELTA_IDX);
    to_write.band_powers[CH7_IDX].theta = bandpwer_ch7.mean_row(THETA_IDX);
    to_write.band_powers[CH7_IDX].alpha = bandpwer_ch7.mean_row(ALPHA_IDX);
    to_write.band_powers[CH7_IDX].beta = bandpwer_ch7.mean_row(BETA_IDX);
    LOG_INF("delta: %f, theta: %f, alpha: %f, beta: %f",
        to_write.band_powers[CH7_IDX].delta,
        to_write.band_powers[CH7_IDX].theta,
        to_write.band_powers[CH7_IDX].alpha,
        to_write.band_powers[CH7_IDX].beta
    );

    // CH8
    bandpwer_ch8.prettyPrint();
    to_write.band_powers[CH8_IDX].delta = bandpwer_ch8.mean_row(DELTA_IDX);
    to_write.band_powers[CH8_IDX].theta = bandpwer_ch8.mean_row(THETA_IDX);
    to_write.band_powers[CH8_IDX].alpha = bandpwer_ch8.mean_row(ALPHA_IDX);
    to_write.band_powers[CH8_IDX].beta = bandpwer_ch8.mean_row(BETA_IDX);
    LOG_INF("delta: %f, theta: %f, alpha: %f, beta: %f",
        to_write.band_powers[CH8_IDX].delta,
        to_write.band_powers[CH8_IDX].theta,
        to_write.band_powers[CH8_IDX].alpha,
        to_write.band_powers[CH8_IDX].beta
    );
}
