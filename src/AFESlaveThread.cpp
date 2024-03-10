#include "AFESlaveThread.h"

LOG_MODULE_REGISTER(eegals_app_core_afeslave, LOG_LEVEL_DBG);
K_THREAD_STACK_DEFINE(afe_slave_stack_area, AFE_SLAVE_THREAD_STACK_SIZE_B);
struct k_thread afe_slave_thread_data;

// SPI slave defines
uint8_t AFESlaveThread::slave_counter = 0;
k_poll_signal AFESlaveThread::spi_slave_done_sig = K_POLL_SIGNAL_INITIALIZER(spi_slave_done_sig);
const device* AFESlaveThread::spi_slave_dev = 
    static_cast<const device*>(
        DEVICE_DT_GET(AFE_SLAVE_SPI)
    );
spi_config AFESlaveThread::spi_slave_cfg = {
	.frequency = AFE_SPI_FREQUENCY_HZ,
	.operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB |
				 SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_OP_MODE_SLAVE,
	.slave = 0,
};
 
AFESlaveThread::AFESlaveThread() {
	if(!device_is_ready(spi_slave_dev)) {
		printk("SPI slave device not ready!\n");
	}
    slave_counter = 0;
};

void AFESlaveThread::Initialize() {
    LOG_DBG("AFESlave::%s -- initializing AFE Wrapper", __FUNCTION__);

    if (id == nullptr) {
        LOG_DBG("AFESlave::%s -- making thread", __FUNCTION__);
        id = k_thread_create(
            &afe_slave_thread_data, afe_slave_stack_area,
            K_THREAD_STACK_SIZEOF(afe_slave_stack_area),
            AFESlaveThread::RunThreadSequence,
            this, NULL, NULL,
            AFE_SLAVE_THREAD_PRIORITY, 0, K_NO_WAIT
        );

        k_thread_name_set(id, "AFESlaveThread");
        LOG_DBG("AFESlave::%s -- thread create successful", __FUNCTION__);
    }
};

void AFESlaveThread::TestSPIWrite() {
    static uint8_t counter = 0;

    const struct spi_buf s_tx_buf = {
        .buf = slave_tx_buffer,
        .len = sizeof(slave_tx_buffer)
    };
    const struct spi_buf_set s_tx = {
        .buffers = &s_tx_buf,
        .count = 1
    };

    struct spi_buf s_rx_buf = {
        .buf = slave_rx_buffer,
        .len = sizeof(slave_rx_buffer),
    };
    const struct spi_buf_set s_rx = {
        .buffers = &s_rx_buf,
        .count = 1
    };

    // Update the TX buffer with a rolling counter
    slave_tx_buffer[1] = counter++;
    printk("SPI SLAVE TX: 0x%.2x, 0x%.2x\n", slave_tx_buffer[0], slave_tx_buffer[1]);

    // Reset signal
    k_poll_signal_reset(&spi_slave_done_sig);

    // Start transaction
    int error = spi_transceive_async(spi_slave_dev, &spi_slave_cfg, &s_tx, &s_rx, &spi_slave_done_sig);
    if(error != 0){
        printk("SPI slave transceive error: %i\n", error);
        return;
    }
    return;
};

int AFESlaveThread::CheckForSpiMessage(void) {
    unsigned int signaled;
    int result;
    k_poll_signal_check(&spi_slave_done_sig, &signaled, &result);
    if(signaled != 0){
        return 0;
    }
    else return -1;
};

void AFESlaveThread::Run() {
    TestSPIWrite();

    while(true) {
        k_msleep(1000);
        if(CheckForSpiMessage() == 0){
            // Print the last received data
            printk("SPI SLAVE RX:");
            for (size_t i = 0; i < sizeof(slave_rx_buffer); i++) {
                printk(" 0x%.2x", slave_rx_buffer[i]);
            }
            printk("\n");

            // Prepare the next SPI slave transaction
            TestSPIWrite();
        }
    }
};
