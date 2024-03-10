#pragma once

#include <zephyr/logging/log_ctrl.h>
#include <zephyr/logging/log.h>
#include "core/Thread.h"
#include "TIBareMetalWrapper.h"

#define AFE_SLAVE_THREAD_STACK_SIZE_B 2048
#define AFE_SLAVE_THREAD_PRIORITY 4
#define AFE_SLAVE_THREAD_MSG_Q_DEPTH 10

class AFESlaveThread : public Thread<AFE_SLAVE_THREAD_MSG_Q_DEPTH> {
private:
    AFESlaveThread();
    ~AFESlaveThread() = default;

    // need to delete copy cstor and assign optor
    AFESlaveThread(const AFESlaveThread &) = delete;
    AFESlaveThread& operator=(const AFESlaveThread&) = delete;

    uint8_t slave_tx_buffer[2];
    uint8_t slave_rx_buffer[2];
    uint8_t slave_counter;

public:
    enum AFESlaveThreadMessage : uint8_t {
        TEST_WRITE,
        INVALID,
    };

    void Initialize() override;
    void Run() override;

    void TestSPIWrite();
    int CheckForSpiMessage();

    static AFESlaveThread& GetInstance() {
        // NOTE: this method of using static local variable is thread-safe
        // NOTE: this has to be implemented in the child class
        static AFESlaveThread instance;
        return instance;
    }
};