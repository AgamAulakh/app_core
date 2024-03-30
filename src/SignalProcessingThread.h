#pragma once

#include <zephyr/logging/log_ctrl.h>
#include <zephyr/logging/log.h>
#include "core/Thread.h"
#include "DataBufferManager.h"

#define SIG_PROC_THREAD_STACK_SIZE_B 2048
#define SIG_PROC_THREAD_PRIORITY 3
#define SIG_PROC_THREAD_MSG_Q_DEPTH 10

class SignalProcessingThread : public Thread<SIG_PROC_THREAD_MSG_Q_DEPTH> {
private:
    SignalProcessingThread();
    ~SignalProcessingThread() = default;

    // need to delete copy cstor and assign optor
    SignalProcessingThread(const SignalProcessingThread &) = delete;
    SignalProcessingThread& operator=(const SignalProcessingThread&) = delete;

    void ProcessOneEpoch();
    void CleanUp();
public:
    enum SignalProcessingThreadMessage : uint8_t {
        CLEAN_UP = 0,
        PROCESS_EPOCH,
        INVALID,
    };

    void Initialize() override;
    void Run() override;

    static SignalProcessingThread& GetInstance() {
        // NOTE: this method of using static local variable is thread-safe
        // NOTE: this has to be implemented in the child class
        static SignalProcessingThread instance;
        return instance;
    }
};