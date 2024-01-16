#include "DataAcquisitionThread.h"

void DataAcquisitionThread::Initialize() {
    if (id == nullptr) {
        id = k_thread_create(
            &data_acq_thread_data, data_acq_stack_area,
            K_THREAD_STACK_SIZEOF(data_acq_stack_area),
            DataAcquisitionThread::RunThreadSequence,
            this, NULL, NULL,
            DATA_ACQ_THREAD_PRIORITY, 0, K_NO_WAIT
        );

        k_thread_name_set(id, "DataAcquisitionThread");
    }
}

void DataAcquisitionThread::Run() {
    uint8_t message = 0;
    while (true) {
        if (message_queue.get(message)) {
            switch (static_cast<DataAcquisitionThreadMessage>(message)) {
                case STOP_READING_AFE:
                case START_READING_AFE:
                case INVALID:
                default:
                    break;
            }
        }
    }
}
