#pragma once

#include <zephyr/logging/log_ctrl.h>
#include <zephyr/logging/log.h>
#include "core/Thread.h"

#define STATE_MACHINE_THREAD_STACK_SIZE 2048
#define STATE_MACHINE_THREAD_PRIORITY 3
#define STATE_MACHINE_THREAD_MSG_Q_DEPTH 0

class StateMachineThread : public Thread<STATE_MACHINE_THREAD_MSG_Q_DEPTH> {
private:
    StateMachineThread();
    ~StateMachineThread() = default;

    // need to delete copy cstor and assign optor
    StateMachineThread(const StateMachineThread &) = delete;
    StateMachineThread& operator=(const StateMachineThread&) = delete;

public:
    void Initialize() override;
    void Run() override;

    static StateMachineThread& GetInstance() {
        // NOTE: this method of using static local variable is thread-safe
        // NOTE: this has to be implemented in the child class
        static StateMachineThread instance;
        return instance;
    }
};