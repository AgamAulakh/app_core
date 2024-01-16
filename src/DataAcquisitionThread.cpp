#include "DataAcquisitionThread.h"

LOG_MODULE_REGISTER(eegals_app_core_daq, LOG_LEVEL_DBG);
K_THREAD_STACK_DEFINE(data_acq_stack_area, DATA_ACQ_THREAD_STACK_SIZE_B);
struct k_thread data_acq_thread_data;

DataAcquisitionThread::DataAcquisitionThread() {
    // todo
    LOG_DBG("DataAcq::%s -- constructor called", __FUNCTION__);
}

void DataAcquisitionThread::Initialize() {
    LOG_DBG("DataAcq::%s -- making thread", __FUNCTION__);

    if (id == nullptr) {
        LOG_DBG("DataAcq::%s -- id == nullptr passed", __FUNCTION__);
        id = k_thread_create(
            &data_acq_thread_data, data_acq_stack_area,
            K_THREAD_STACK_SIZEOF(data_acq_stack_area),
            DataAcquisitionThread::RunThreadSequence,
            this, NULL, NULL,
            DATA_ACQ_THREAD_PRIORITY, 0, K_NO_WAIT
        );

        LOG_DBG("DataAcq::%s -- thread create successful", __FUNCTION__);

        k_thread_name_set(id, "DataAcquisitionThread");

        LOG_DBG("DataAcq::%s -- set name successful", __FUNCTION__);
    }
}

void DataAcquisitionThread::Run() {
    uint8_t message = 0;
    while (true) {
        // if (message_queue.get(message)) {
        //     switch (static_cast<DataAcquisitionThreadMessage>(message)) {
        //         case STOP_READING_AFE:
        //         case START_READING_AFE:
        //         case INVALID:
        //         default:
        //             break;
        //     }
        // }
		LOG_DBG("DataAcq::%s -- up time %u ms", __FUNCTION__, k_uptime_get_32());
		k_msleep(2000);
    }
}
