#include "DataBufferManager.h"
// log level declaration
LOG_MODULE_REGISTER(data_buffer_manager, LOG_LEVEL_DBG);

ring_buf DataBufferManager::buffer;
sample_t DataBufferManager::data_buffer[max_samples_ring_buffer] = {0};
Semaphore DataBufferManager::buffer_lock = Semaphore();

DataBufferManager::DataBufferManager(){
    ring_buf_init(&buffer, total_size_ring_buffer_B, reinterpret_cast<uint8_t*>(data_buffer));
}

// ring functionality
bool DataBufferManager::WriteOneSample(const sample_t &sample) {
    uint32_t samples_written_B = ring_buf_put(&buffer,
        const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(&sample)),
        sample_size_B
    ) == sample_size_B;

    LOG_INF("DataBufferManager::%s bytes used: %u", __FUNCTION__, samples_written_B);

    if ((samples_written_B / sample_size_B) == num_samples_per_epoch) {
        // collected one full epoch, signal to data acq and sig proc thread
        // DATABUFFER CANT KNOW ABOUT ANY OF THE THREADS!!
        // DataAcquisitionThread::GetInstance().SendMessage(
        //     DataAcquisitionThread::INCREASE_EPOCH_COUNT
        // );
        // SignalProcessingThread::GetInstance().SendMessage(
        //     SignalProcessingThread::PROCESS_EPOCH
        // );
    }
    return (samples_written_B == sample_size_B);
};

bool DataBufferManager::ReadOneSample(sample_t &sample) {
    return ring_buf_get(&buffer,
        const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(&sample)),
        sample_size_B
    ) == sample_size_B;
};

DataBufferManager::ReadEpoch(ArmMatrixWrapper<num_electrodes, num_samples_per_epoch> &mat) {
    sample_t to_read = { 0 };
    for(uint32_t i = 0; i < num_samples_per_epoch; i++) {
        ReadOneSample(to_read);
        mat.set_at(to_read.ch1, CH1_IDX, i);
        mat.set_at(to_read.ch2, CH2_IDX, i);
        mat.set_at(to_read.ch3, CH3_IDX, i);
        mat.set_at(to_read.ch4, CH4_IDX, i);
        mat.set_at(to_read.ch5, CH5_IDX, i);
        mat.set_at(to_read.ch6, CH6_IDX, i);
        mat.set_at(to_read.ch7, CH7_IDX, i);
        mat.set_at(to_read.ch8, CH8_IDX, i);
    }
};

// helpers
bool DataBufferManager::IsEmpty() {
    return ring_buf_is_empty(&buffer);
};
size_t DataBufferManager::GetFreeSpace() {
    return ring_buf_space_get(&buffer);
};
size_t DataBufferManager::GetUsedSpace() {
    return total_size_ring_buffer_B - ring_buf_space_get(&buffer);
};
void DataBufferManager::ResetBuffer() {
    memset(data_buffer, 0, sizeof(data_buffer));
};