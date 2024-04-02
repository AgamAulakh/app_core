#pragma once

#include <zephyr/logging/log_ctrl.h>
#include <zephyr/logging/log.h>
#include "core/Thread.h"
#include "TIBareMetalWrapper.h"

#define DATA_ACQ_THREAD_STACK_SIZE_B 2048
#define DATA_ACQ_THREAD_PRIORITY 4 // max based on prj config
#define DATA_ACQ_THREAD_MSG_Q_DEPTH 10

// #define AFE_RST_PIN DT_GPIO_PIN(AFE_RESET, gpios)

class DataAcquisitionThread : public Thread<DATA_ACQ_THREAD_MSG_Q_DEPTH> {
private:
    // TIFrontEndWrapper AFEWrapper;
    DataAcquisitionThread();
    ~DataAcquisitionThread() = default;

    // need to delete copy cstor and assign optor
    DataAcquisitionThread(const DataAcquisitionThread &) = delete;
    DataAcquisitionThread& operator=(const DataAcquisitionThread&) = delete;

    // ADS1299Driver afe_driver;
    TIBareMetalWrapper AFEWrapper;
public:
    enum DataAcquisitionThreadMessage : uint8_t {
        STOP_READING_AFE = 0,
        START_READING_AFE,
        READ_AFE_SAMPLE,
        RESET_AFE,
        CHECK_AFE_REGISTERS,
        RUN_INPUT_SHORT_TEST,
        RUN_INTERNAL_SQUARE_WAVE_TEST_SMALL_SLOW,
        RUN_INTERNAL_SQUARE_WAVE_TEST_BIG_FAST,
        INVALID,
    };

    void Initialize() override;
    void Run() override;

    static DataAcquisitionThread& GetInstance() {
        // NOTE: this method of using static local variable is thread-safe
        // NOTE: this has to be implemented in the child class
        static DataAcquisitionThread instance;
        return instance;
    }
};