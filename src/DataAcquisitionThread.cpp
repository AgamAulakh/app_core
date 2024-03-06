#include "DataAcquisitionThread.h"

LOG_MODULE_REGISTER(eegals_app_core_daq, LOG_LEVEL_DBG);
K_THREAD_STACK_DEFINE(data_acq_stack_area, DATA_ACQ_THREAD_STACK_SIZE_B);
struct k_thread data_acq_thread_data;

// const struct device* afe_reset_device = const_cast<struct device *>(DEVICE_DT_GET(AFE_RESET_DEV));
// device* afe_reset_device = device_get_binding(DT_GPIO_LABEL(AFE_RESET, gpios));

DataAcquisitionThread::DataAcquisitionThread() {
    AFEWrapper = TIBareMetalWrapper();
    // set up data manager to listen to AFE
    // DataBufferManager::spi_dma_setup(spi_dev);

    // if (!device_is_ready(afe_spi_global_device)) {
    //     LOG_ERR("DataAcq::%s -- SPI device not ready!", __FUNCTION__);
    //     return;
    // }

    // if (!gpio_is_ready_dt(&led)) {
	// 	return;
	// }

	// int ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	// if (ret < 0) {
	// 	return;
	// }

    // ResetSPI();
    // TestSPI();
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
    k_msleep(200); // tpor >= 150 ms
    
    // gpio_pin_toggle_dt(&led);
    k_msleep(1); // trst >= 1 us

    
    // gpio_pin_toggle_dt(&led);
    k_msleep(1); // t >= 9us

    LOG_INF("reset afe with hardware pin. afe in RDATAC mode. checking id...");
    // afe_driver.ads1299_check_id();
}

void DataAcquisitionThread::ResetSPIBareMetal() {
    AFEWrapper.Start();
}

void DataAcquisitionThread::TestSPI() {
    // afe_driver.ads1299_stop_rdatac();
    // afe_driver.ads1299_check_id();
    // afe_driver.ads1299_init_regs();
    // afe_driver.ads1299_start_rdatac();
}

void DataAcquisitionThread::Run() {
    // set AFE in continuous read mode
    uint8_t message = 0;
    while (true) {
        if (message_queue.get_with_blocking_wait(message)) {
            uint8_t message_enum = static_cast<DataAcquisitionThreadMessage>(message);
		    LOG_DBG("DataAcq::%s -- received message: %u at: %u ms", __FUNCTION__, message_enum, k_uptime_get_32());
            switch (message_enum) {
                case STOP_READING_AFE:
                    // AFEWrapper.Stop();
                    break;
                case START_READING_AFE:
                    break;
                case RESET_AFE:
                    ResetSPIBareMetal();
                    break;
                case INVALID:
                    break;
                default:
                    break;
            }
        }
    }
}
