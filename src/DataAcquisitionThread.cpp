#include "DataAcquisitionThread.h"

void DataAcquisitionThread::Initialize() {
    if (thread_id == nullptr) {
        // thread doesnt exist, make one naow:
        thread_id = k_thread_create(
            K_THREAD_STACK_ALLOC(daq_thread_stack, DATA_ACQUISITION_THREAD_STACK_SIZE_B),
            K_THREAD_STACK_SIZEOF(daq_thread_stack),
            Run,
            this, // NEED to pass this ptr because it doesnt exist in the static function
            nullptr,
            nullptr,
            K_PRIO_COOP(7), // TODO: change
            0,
            K_NO_WAIT);

        k_thread_name_set(thread_id, "DataAcquisitionThread");

        if (thread_id == nullptr) {
            // TODO: print err on debug uart
        }
    }
}
void DataAcquisitionThread::Run() {
    while (true) {
        ThreadMessage msg;
        if (k_msgq_get(&message_queue, &msg, K_FOREVER) == 0) {
            switch(msg.data_acq_msg) {
                STOP_READING_AFE:
                START_READING_AFE:
                INVALID:
                    break;
            }
        }
    }
}
