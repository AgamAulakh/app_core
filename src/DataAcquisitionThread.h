#ifndef DATA_ACQUISITION_THREAD_H
#define DATA_ACQUISITION_THREAD_H

#include "core/Thread.h"

#define DATA_ACQ_THREAD_STACK_SIZE_B 1024
#define DATA_ACQ_THREAD_PRIORITY 5
#define DATA_ACQ_THREAD_MSG_Q_DEPTH 10

K_THREAD_STACK_DEFINE(data_acq_stack_area, DATA_ACQ_THREAD_STACK_SIZE_B);
struct k_thread data_acq_thread_data;

class DataAcquisitionThread : public Thread<DATA_ACQ_THREAD_MSG_Q_DEPTH> {
private:
    DataAcquisitionThread();
    ~DataAcquisitionThread() = default;

    // need to delete copy cstor and assign optor
    DataAcquisitionThread(const DataAcquisitionThread &) = delete;
    DataAcquisitionThread& operator=(const DataAcquisitionThread&) = delete;

public:
    enum DataAcquisitionThreadMessage : uint8_t {
        STOP_READING_AFE = 0,
        START_READING_AFE,
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

#endif