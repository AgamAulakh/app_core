#ifndef DATA_ACQUISITION_THREAD_H
#define DATA_ACQUISITION_THREAD_H

#include "core/Thread.h"

#define DATA_ACQUISITION_THREAD_STACK_SIZE_B 1024

class DataAcquisitionThread : public Thread {
private:
    k_tid_t thread_id;
    DataAcquisitionThread();

public:
    void Initialize() override;
    void Run() override;

    // NOTE: using singleton
    static DataAcquisitionThread& GetInstance() {
        // NOTE: this method of using static local variable is thread-safe
        static DataAcquisitionThread instance;
        return instance;
    }
};

#endif