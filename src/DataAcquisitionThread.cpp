#include "DataAcquisitionThread.h"

LOG_MODULE_REGISTER(eegals_app_core_daq, LOG_LEVEL_DBG);
K_THREAD_STACK_DEFINE(data_acq_stack_area, DATA_ACQ_THREAD_STACK_SIZE_B);
struct k_thread data_acq_thread_data;
device* afe_spi_device = const_cast<struct device *>(DEVICE_DT_GET(AFE_SPI));
spi_config afe_spi_config = {
    ADS1299_SPI_FREQUENCY_HZ,
    (SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8)),
    0
};

const struct device* afe_reset_device = const_cast<struct device *>(DEVICE_DT_GET(AFE_RESET_DEV));
// device* afe_reset_device = device_get_binding(DT_GPIO_LABEL(AFE_RESET, gpios));

DataAcquisitionThread::DataAcquisitionThread() : afe_driver(afe_spi_device, &afe_spi_config) {
    // AFEWrapper = TIFrontEndWrapper();
    // set up data manager to listen to AFE
    // DataBufferManager::spi_dma_setup(spi_dev);
    if (!device_is_ready(afe_spi_device)) {
        LOG_ERR("DataAcq::%s -- SPI device not ready!", __FUNCTION__);
        return;
    }

    if (afe_reset_device == NULL || !device_is_ready(afe_reset_device)) {
    LOG_ERR("AFE device doesnt exissst");
    }

    int err = gpio_pin_configure(afe_reset_device, AFE_RESET_PIN, AFE_RESET_FLAGS);
    if (err != 0) {
        LOG_ERR("COULD NOT CONFIGURE AS GPIO");
    }

    ResetSPI();
    TestSPI();
}

void DataAcquisitionThread::Initialize() {
    LOG_DBG("DataAcq::%s -- initializing AFE Wrapper", __FUNCTION__);
    // AFEWrapper.Initialize();

    if (id == nullptr) {
        LOG_DBG("DataAcq::%s -- making thread", __FUNCTION__);
        id = k_thread_create(
            &data_acq_thread_data, data_acq_stack_area,
            K_THREAD_STACK_SIZEOF(data_acq_stack_area),
            DataAcquisitionThread::RunThreadSequence,
            this, NULL, NULL,
            DATA_ACQ_THREAD_PRIORITY, 0, K_NO_WAIT
        );

        k_thread_name_set(id, "DataAcquisitionThread");
        LOG_DBG("DataAcq::%s -- thread create successful", __FUNCTION__);
    }
}

void DataAcquisitionThread::ResetSPI() {
    // Get the GPIO device pointer for the LED pin

    // afe_driver.ads1299_powerup_reset();

    gpio_pin_set(afe_reset_device, AFE_RESET_PIN, 0);
    k_msleep(1000);
    gpio_pin_set(afe_reset_device, AFE_RESET_PIN, 1);
    k_msleep(1000);
    LOG_INF("reset afe with hardware pin");
}

void DataAcquisitionThread::TestSPI() {
    afe_driver.ads1299_wake();
    afe_driver.ads1299_check_id();
    afe_driver.ads1299_init_regs();
    afe_driver.ads1299_start_rdatac();
}

void DataAcquisitionThread::Run() {
    // set AFE in continuous read mode
    uint8_t message = 0;
    int a = 0;
    int b = 0;
    int c = 0;
    int d = 0;
    while (true) {
        if (message_queue.get_with_blocking_wait(message)) {
            uint8_t message_enum = static_cast<DataAcquisitionThreadMessage>(message);
		    LOG_DBG("DataAcq::%s -- received message: %u at: %u ms", __FUNCTION__, message_enum, k_uptime_get_32());
            switch (message_enum) {
                case STOP_READING_AFE:
                    // AFEWrapper.Stop();
                    break;
                case START_READING_AFE:
                    a = 0;
                    b = 0;
                    c = 0;
                    d = 0;        
                    afe_driver.get_eeg_voltage_samples(&a, &b, &c, &d);
                    LOG_INF("afe read: %d, %d, %d, %d", a, b, c, d);
                    // AFEWrapper.Initialize();
                    // AFEWrapper.Start();
                    break;
                case INVALID:
                    break;
                default:
                    break;
            }
        }
    }
}
