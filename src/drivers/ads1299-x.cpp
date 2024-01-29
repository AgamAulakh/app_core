/* Copyright (c) 2016 Musa Mahmood
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "ads1299-x.h"
#include <stdio.h>
#include <string.h>
// #include "compiler_abstraction.h"
// #include "nrf.h"

LOG_MODULE_REGISTER(ads1299_driver, LOG_LEVEL_DBG);

/**@TX,RX Stuff: */
#define TX_RX_MSG_LENGTH         				7

/**@DEBUG STUFF */
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 
#define BYTE_TO_BINARY_PATTERN_16BIT "%c%c%c%c %c%c%c%c %c%c%c%c %c%c%c%c\r\n"
#define BYTE_TO_BINARY_16BIT(byte)  \
	(byte & 0x8000 ? '1' : '0'), \
	(byte & 0x4000 ? '1' : '0'), \
	(byte & 0x2000 ? '1' : '0'), \
	(byte & 0x1000 ? '1' : '0'), \
	(byte & 0x800 ? '1' : '0'), \
	(byte & 0x400 ? '1' : '0'), \
	(byte & 0x200 ? '1' : '0'), \
	(byte & 0x100 ? '1' : '0'), \
	(byte & 0x80 ? '1' : '0'), \
	(byte & 0x40 ? '1' : '0'), \
	(byte & 0x20 ? '1' : '0'), \
	(byte & 0x10 ? '1' : '0'), \
	(byte & 0x08 ? '1' : '0'), \
	(byte & 0x04 ? '1' : '0'), \
	(byte & 0x02 ? '1' : '0'), \
	(byte & 0x01 ? '1' : '0')

/**
8 \Note: ADS1299 Default Registers
*/

uint8_t ads1299_default_registers[] = {
		ADS1299_REGDEFAULT_CONFIG1,
		ADS1299_REGDEFAULT_CONFIG2,
		ADS1299_REGDEFAULT_CONFIG3,
		ADS1299_REGDEFAULT_LOFF,
		ADS1299_REGDEFAULT_CH1SET,
		ADS1299_REGDEFAULT_CH2SET,
 		ADS1299_REGDEFAULT_CH3SET,
 		ADS1299_REGDEFAULT_CH4SET,
 		ADS1299_REGDEFAULT_CH5SET,
 		ADS1299_REGDEFAULT_CH6SET,
 		ADS1299_REGDEFAULT_CH7SET,
 		ADS1299_REGDEFAULT_CH8SET,
		ADS1299_REGDEFAULT_BIAS_SENSP,
		ADS1299_REGDEFAULT_BIAS_SENSN,
		ADS1299_REGDEFAULT_LOFF_SENSP,
		ADS1299_REGDEFAULT_LOFF_SENSN,
		ADS1299_REGDEFAULT_LOFF_FLIP,
		ADS1299_REGDEFAULT_LOFF_STATP,
		ADS1299_REGDEFAULT_LOFF_STATN,
		ADS1299_REGDEFAULT_GPIO,
		ADS1299_REGDEFAULT_MISC1,
		ADS1299_REGDEFAULT_MISC2,
		ADS1299_REGDEFAULT_CONFIG4
};

/*
 * SPI Transceive Wrapper:
 */
void ADS1299Driver::ads1299_spi_transfer(uint8_t* tx_data, size_t tx_len, uint8_t* rx_data, size_t rx_len) {
	// current behaviour: function will continue to transmit despite errors in tx/rx buf
	struct spi_buf tx_buf = { .buf = NULL, .len = 0 };
	struct spi_buf rx_buf = { .buf = NULL, .len = 0 };

	struct spi_buf_set tx_buf_set = { .buffers = NULL, .count = 0,};
    struct spi_buf_set rx_buf_set = { .buffers = NULL, .count = 0,};

	if (tx_data != NULL && tx_len > 0) {
		tx_buf.buf = tx_data;
		tx_buf.len = tx_len;
		tx_buf_set.buffers = &tx_buf;
	}
	if (rx_data != NULL && rx_len > 0) {
		rx_buf.buf = rx_data;
		rx_buf.len = rx_len;
		rx_buf_set.buffers = &rx_buf;
	}

    int ret = spi_transceive(spi_dev, spi_cfg, &tx_buf_set, &rx_buf_set);
    if (ret) {
        LOG_ERR("SPI transfer failed with error %d", ret);
        return;
    }
};

/*
 * ADS1299 CONTROL FUNCTIONS:
 */
void ADS1299Driver::ads1299_init_regs(void) {
    uint8_t num_registers = 23;
    uint8_t txrx_size = num_registers + 2;
    uint8_t wreg_init_opcode = 0x41;
    uint8_t tx_data_spi[txrx_size] = { 0 };
    uint8_t rx_data_spi[txrx_size] = { 0 };

    tx_data_spi[0] = wreg_init_opcode;
    tx_data_spi[1] = num_registers - 1;

    for (int j = 0; j < num_registers; ++j) {
        tx_data_spi[j + 2] = ads1299_default_registers[j];
    }

    ads1299_spi_transfer(tx_data_spi, txrx_size, rx_data_spi, txrx_size);
    k_msleep(150);
    LOG_INF("Power-on reset and initialization procedure..");
};

void ADS1299Driver::ads1299_powerup_reset(void)
{
	#if defined(BOARD_PCA10028) | defined(BOARD_NRF_BREAKOUT)
		nrf_gpio_pin_clear(ADS1299_PWDN_RST_PIN);
	#endif
	#if defined(BOARD_FULL_EEG_V1)
		nrf_gpio_pin_clear(ADS1299_RESET_PIN);
		nrf_gpio_pin_clear(ADS1299_PWDN_PIN);
	#endif
	k_msleep(50);
	LOG_DBG(" ADS1299-x POWERED UP AND RESET..\r\n");
};

void ADS1299Driver::ads1299_powerdn(void)
{
	#if defined(BOARD_PCA10028) | defined(BOARD_NRF_BREAKOUT)
		nrf_gpio_pin_clear(ADS1299_PWDN_RST_PIN);
	#endif
	#if defined(BOARD_FULL_EEG_V1)
		nrf_gpio_pin_clear(ADS1299_RESET_PIN);
		nrf_gpio_pin_clear(ADS1299_PWDN_PIN);
	#endif
	k_msleep(20);
	LOG_DBG(" ADS1299-x POWERED DOWN..\r\n");
};

void ADS1299Driver::ads1299_powerup(void)
{
	#if defined(BOARD_PCA10028) | defined(BOARD_NRF_BREAKOUT)
		nrf_gpio_pin_set(ADS1299_PWDN_RST_PIN);
	#endif
	#if defined(BOARD_FULL_EEG_V1)
		nrf_gpio_pin_set(ADS1299_RESET_PIN);
		nrf_gpio_pin_set(ADS1299_PWDN_PIN);
	#endif
	k_msleep(1000);		// Allow time for power-on reset
	LOG_DBG(" ADS1299-x POWERED UP...\r\n");
};

void ADS1299Driver::ads1299_standby(void) {
    uint8_t tx_data_spi = ADS1299_OPC_STANDBY;
    uint8_t rx_data_spi;

    ads1299_spi_transfer(&tx_data_spi, 1, &rx_data_spi, 1);
    LOG_DBG("ADS1299-x placed in standby mode...");
};

void ADS1299Driver::ads1299_wake(void) {
	uint8_t tx_data_spi = ADS1299_OPC_WAKEUP;
	uint8_t rx_data_spi;

    ads1299_spi_transfer(&tx_data_spi, 1, &rx_data_spi, 1);
	k_msleep(10); // Allow time to wake up - 10ms
	LOG_DBG(" ADS1299-x Wakeup..");
};

void ADS1299Driver::ads1299_soft_start_conversion(void) {
	uint8_t tx_data_spi = ADS1299_OPC_START;
	uint8_t rx_data_spi;

    ads1299_spi_transfer(&tx_data_spi, 1, &rx_data_spi, 1);
	LOG_DBG(" Start ADC conversion..");
};

void ADS1299Driver::ads1299_stop_rdatac(void) {
	uint8_t tx_data_spi = ADS1299_OPC_SDATAC;
	uint8_t rx_data_spi;

    ads1299_spi_transfer(&tx_data_spi, 1, &rx_data_spi, 1);
	LOG_DBG(" Continuous Data Output Disabled..");
};

void ADS1299Driver::ads1299_start_rdatac(void) {
	uint8_t tx_data_spi = ADS1299_OPC_RDATAC;
	uint8_t rx_data_spi;

	ads1299_spi_transfer(&tx_data_spi, 1, &rx_data_spi, 1);
	LOG_DBG(" Continuous Data Output Enabled..");
};

void ADS1299Driver::ads1299_check_id(void) {
	uint8_t device_id_reg_value;
	uint8_t tx_data_spi[3];
	uint8_t rx_data_spi[7];
	tx_data_spi[0] = 0x20;	//Request Device ID
	tx_data_spi[1] = 0x01;	//Intend to read 1 byte
	tx_data_spi[2] = 0x00;	//This will be replaced by Reg Data

	ads1299_spi_transfer(tx_data_spi, 2+tx_data_spi[1], rx_data_spi, 2+tx_data_spi[1]);

	k_msleep(20); //Wait for response:
	device_id_reg_value = rx_data_spi[2];
	bool is_ads_1299_4 = (device_id_reg_value & 0x1F) == (ADS1299_4_DEVICE_ID);
	bool is_ads_1299_6 = (device_id_reg_value & 0x1F) == (ADS1299_6_DEVICE_ID);
	bool is_ads_1299	 = (device_id_reg_value & 0x1F) == (ADS1299_DEVICE_ID);

	uint8_t revisionVersion = (device_id_reg_value & 0xE0)>>5;
	if (is_ads_1299||is_ads_1299_6||is_ads_1299_4) {
		LOG_DBG("Device Address Matches!");
	} else {
		LOG_DBG("********SPI I/O Error, Device Not Detected! ***********");
		LOG_DBG("SPI Transfer Dump:");
		LOG_DBG("ID[b0->2]: [0x%x | 0x%x | 0x%x]", rx_data_spi[0],rx_data_spi[1],rx_data_spi[2]);
		LOG_DBG("ID[b3->6]: [0x%x | 0x%x | 0x%x | 0x%x]", rx_data_spi[3],rx_data_spi[4],rx_data_spi[5],rx_data_spi[6]);
	}
	if (is_ads_1299) {
		LOG_DBG("Device Name: ADS1299");
	} else if (is_ads_1299_6) {
		LOG_DBG("Device Name: ADS1299-6");
	} else if (is_ads_1299_4) {
		LOG_DBG("Device Name: ADS1299-4");
	} 
	if (is_ads_1299||is_ads_1299_6||is_ads_1299_4) {
		LOG_DBG("Device Revision #%d",revisionVersion);
		LOG_DBG("Device ID: 0x%x",device_id_reg_value);
	}
};

/* DATA RETRIEVAL FUNCTIONS **********************************************************************************************************************/

/**@brief Function for acquiring a EEG Voltage Measurement samples.
 *
 * @details Uses SPI
 *          
 */
void ADS1299Driver::get_eeg_voltage_samples(int32_t *eeg1, int32_t *eeg2, int32_t *eeg3, int32_t *eeg4) {
		uint8_t tx_rx_data[15] = {
			0x00, 0x00, 0x00,
			0x00, 0x00, 0x00,
			0x00, 0x00, 0x00,
			0x00, 0x00, 0x00,
			0x00, 0x00, 0x00
		};
	
		ads1299_spi_transfer(tx_rx_data, 15, tx_rx_data, 15);

		uint8_t cnt = 0;
		do {
			if(tx_rx_data[0]==0xC0){
				*eeg1 =  ( (tx_rx_data[3] << 16) | (tx_rx_data[4] << 8) | (tx_rx_data[5]) );					
				*eeg2 =  ( (tx_rx_data[6] << 16) | (tx_rx_data[7] << 8) | (tx_rx_data[8]) );			
				*eeg3 =  ( (tx_rx_data[9] << 16) | (tx_rx_data[10] << 8) | (tx_rx_data[11]) );
				*eeg4 =  ( (tx_rx_data[12] << 16) | (tx_rx_data[13] << 8) | (tx_rx_data[14]) );
				break;
			}
			cnt++;
			k_msleep(1);
		} while(cnt<255);
		//LOG_DBG("B0-2 = [0x%x 0x%x 0x%x | cnt=%d]\r\n",tx_rx_data[0],tx_rx_data[1],tx_rx_data[2],cnt);
		//LOG_DBG("DATA:[0x%x 0x%x 0x%x 0x%x]\r\n",*eeg1,*eeg2,*eeg3,*eeg4);
};

// // // // // //
// End of File //
// // // // // // 
