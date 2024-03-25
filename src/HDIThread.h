#pragma once
// #include "display_st7735r.h"
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
#include <lvgl.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include "core/Thread.h"

#define HDI_THREAD_STACK_SIZE_B 512
#define HDI_THREAD_PRIORITY -1
#define HDI_THREAD_MSG_Q_DEPTH 100

#define LCD_SPI DT_NODELABEL(lcd_spi)
#define LCD_SPI_CS_DEV DT_PHANDLE(LCD_SPI, cs_gpios)
#define LCD_SPI_CD_PIN DT_PHA(LCD_SPI, cs_gpios, pin)
#define LCD_SPI_CS_DT_SPEC SPI_CS_GPIOS_DT_SPEC_GET(DT_NODELABEL(ref_lcd_spi))
#define LCD_SPI_FREQUENCY_hZ 2000000

class HDIThread : public Thread<HDI_THREAD_MSG_Q_DEPTH>  {
private:
    HDIThread();
    ~HDIThread() = default;

    HDIThread(const HDIThread &) = delete;
    HDIThread& operator=(const HDIThread) = delete;

    static const struct device* spi_dev;
    static const struct device* display;
    static struct spi_config spi_cfg;
    static struct k_poll_signal spi_done_sig;
    static lv_obj_t* screen;

    // arguments
    static int dev_state;
    static float power[4];

    static uint32_t count;

public:
    static HDIThread& GetInstance() {
        static HDIThread instance;
        return instance;
    };
    
    enum HDIThreadMessage : uint8_t {
        DISPLAY_RESULTS = 0,
        DISPLAY_ERROR,
        DISPLAY_STARTUP,
        DISPLAY_INSTRUCTIONS,
        DISPLAY_RESET,
        DISPLAY_BATTERY,
        DISPLAY_HELLO_WORLD,
        INVALID,
    };

    void Initialize() override;
    void Run() override;

    static void DisplayDefaultMessage();
    static void DisplayMessage(const char* message);
    static void DisplayResults(float *alphaPower, float *betaPower, float *deltaPower, float *thetaPower);
};