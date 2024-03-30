#include "SignalProcessingThread.h"

LOG_MODULE_REGISTER(eegals_app_core_sigproc, LOG_LEVEL_DBG);
K_THREAD_STACK_DEFINE(sig_proc_stack_area, SIG_PROC_THREAD_STACK_SIZE_B);
struct k_thread sig_proc_thread_data;

SignalProcessingThread::SignalProcessingThread() {

}

void SignalProcessingThread::Initialize() {
    LOG_DBG("SigProc::%s -- initializing AFE Wrapper", __FUNCTION__);

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
}

void SignalProcessingThread::ProcessOneEpoch() {
    LOG_DBG("SigProc::%s called", __FUNCTION__);
    // wait until we databuffer isn't being written to
    DataBufferManager::buffer_lock.wait();

    ArmMatrixWrapper<num_electrodes, num_samples_per_epoch> epoch_mat;
    DataBufferManager::ReadEpoch(epoch_mat);
    epoch_mat.pretty_print();

    // give back databuffer access
    DataBufferManager::buffer_lock.give();
};

void SignalProcessingThread::CleanUp() {
    LOG_DBG("SigProc::%s called", __FUNCTION__);
};

void SignalProcessingThread::Run() {
    // set AFE in continuous read mode
    uint8_t message = 0;
    while (true) {
        if (message_queue.get_with_blocking_wait(message)) {
            uint8_t message_enum = static_cast<SignalProcessingThread>(message);
		    LOG_DBG("SigProc::%s -- received message: %u at: %u ms",
                __FUNCTION__,
                message_enum,
                k_uptime_get_32()
            );
            switch (message_enum) {
                case CLEAN_UP:
                    CleanUp();
                    break;
                case PROCESS_EPOCH:
                    ProcessOneEpoch();
                    break;
                default:
                    break;
            }
        }
    }
}