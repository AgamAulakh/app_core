#ifndef THREAD_COMMAND_H
#define THREAD_COMMAND_H

#include <stdlib.h>

#define MESSAGE_SIZE_BYTES sizeof(ThreadMessage)

enum class DataAcquisitionThreadMessage : uint8_t {
    STOP_READING_AFE = 0,
    START_READING_AFE,
    INVALID,
};

enum class SignalProcessingThreadMessage : uint8_t {
    STOP_PROCESSING_EEG = 0,
    START_PROCESSING_EEG,
    INVALID,
};

// Combined message type
union ThreadMessage {
    DataAcquisitionThreadMessage data_acq_msg;
    SignalProcessingThreadMessage sig_proc_msg;
    
    // Add types as needed
};

#endif