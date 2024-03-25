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

#define LCD_SPI DT_NODELABEL(lcd_spi)
#define LCD_SPI_CS_DEV DT_PHANDLE(LCD_SPI, cs_gpios)
#define LCD_SPI_CD_PIN DT_PHA(LCD_SPI, cs_gpios, pin)
#define LCD_SPI_CS_DT_SPEC SPI_CS_GPIOS_DT_SPEC_GET(DT_NODELABEL(ref_lcd_spi))
#define LCD_SPI_FREQUENCY_hZ 2000000

class LCDHandler  {
private:
    static const struct device* spi_dev;
    static const struct device* display;
    static struct spi_config spi_cfg;
    static struct k_poll_signal spi_done_sig;
    static lv_obj_t* screen;

public:
    LCDHandler();
    ~LCDHandler() = default;

    static void DisplayDefaultMessage(uint32_t count);
    static void DisplayMessage(const char* message);
    static void DisplayResults(float *alphaPower, float *betaPower, float *deltaPower, float *thetaPower);
};