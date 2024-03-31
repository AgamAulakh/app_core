#include <DataAcquisitionThread.h>

LOG_MODULE_REGISTER(eegals_app_core_daq, LOG_LEVEL_DBG);
K_THREAD_STACK_DEFINE(data_acq_stack_area, DATA_ACQ_THREAD_STACK_SIZE_B);
struct k_thread data_acq_thread_data;

DataAcquisitionThread::DataAcquisitionThread() : AFEWrapper() {
    // set up data manager to listen to AFE
    // DataBufferManager::spi_dma_setup(spi_dev);
}

void DataAcquisitionThread::Initialize() {
    LOG_DBG("DataAcq::%s -- initializing AFE Wrapper", __FUNCTION__);
    AFEWrapper.Initialize();

    if (id == nullptr) {
        LOG_DBG("DataAcq::%s -- making thread", __FUNCTION__);
        id = k_thread_create(
            &data_acq_thread_data, data_acq_stack_area,
            K_THREAD_STACK_SIZEOF(data_acq_stack_area),
            DataAcquisitionThread::RunThreadSequence,
            this, NULL, NULL,
            DATA_ACQ_THREAD_PRIORITY, 0, K_NO_WAIT
        );

        k_thread_name_set(id, "DataAcquisitionThread");
        LOG_DBG("DataAcq::%s -- thread create successful", __FUNCTION__);
    }
}

void DataAcquisitionThread::Run() {
    // set AFE in continuous read mode
    k_msleep(2500);
    AFEWrapper.Initialize();

    // handle incomming messages    
    uint8_t message = 0;
    while (true) {
        if (message_queue.get_with_blocking_wait(message)) {
            uint8_t message_enum = static_cast<DataAcquisitionThreadMessage>(message);
		    LOG_DBG("DataAcq::%s -- received message: %u at: %u ms",
                __FUNCTION__,
                message_enum,
                k_uptime_get_32()
            );
            switch (message_enum) {
                case STOP_READING_AFE:
                    AFEWrapper.Stop();
                    break;
                case START_READING_AFE:
                    AFEWrapper.Start();
                    break;
                case READ_AFE_DEFAULT_EPOCH:
                    AFEWrapper.ReadContinuous();
                    break;
                case READ_AFE_SAMPLE:
                    AFEWrapper.ReadOneSample();
                    break;
                case RESET_AFE:
                    LOG_DBG("DataAcq::%s dangerous request, ignoring reset command", __FUNCTION__);
                    break;
                case CHECK_AFE_REGISTERS:
                    AFEWrapper.CheckAllRegisters();
                    break;
                case RUN_INPUT_SHORT_TEST:
                    AFEWrapper.RunInputShortTest();
                    // because test takes ~2 seconds to run asyncronously, sleep before changing registers
                    k_msleep(2000);
                    AFEWrapper.SetDefaultRegisters();
                    break;
                case RUN_INTERNAL_SQUARE_WAVE_TEST:
                    AFEWrapper.RunInternalSquareWaveTest();
                    // because test takes ~2 seconds to run asyncronously, sleep before changing registers
                    k_msleep(2000);
                    AFEWrapper.SetDefaultRegisters();
                    break;
                case INVALID:
                    break;
                default:
                    break;
            }
        }
    }
}