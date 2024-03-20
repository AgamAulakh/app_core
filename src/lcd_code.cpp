#include "display_st7735r.h"
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/pm/device.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
#include <lvgl.h>
LOG_MODULE_REGISTER(sample, LOG_LEVEL_INF);

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>


// Set text and background colour
//uint16_t text_colour = 0x0000; //Black
//uint16_t background_colour = 0xFFFF //White; 

// set cursors
//uint16_t x = 10;
//uint16_t y = 10;

void displayMessage()
//void displayMessage(const char *message)
{

    const struct device *display_dev;
    struct display_capabilities capabilities;

    //display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    display_dev = device_get_binding(DT_LABEL(DT_INST(0, sitronix_st7735r)));

     if (display_dev == NULL) {
        printk("Device not found\n");
        return;
    }

    lv_obj_t *label = lv_label_create(lv_scr_act(), NULL); //ensure the acfive screen is being used for displaying
    lv_label_set_text(label, message);  //create a text label
    //lv_label_set_text(label, "Hello"); 
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);  // coordinates for placing text
    lv_task_handler(); // continuously updates display based on changes in text...might need to be in a while loop?
    display_blanking_off(display_dev); //keeps the screen on (no blanking)

}


void displayResults(){

    const struct device *display_dev;
    struct display_capabilities capabilities;

    display_dev = device_get_binding(DT_LABEL(DT_INST(0, sitronix_st7735r)));

     if (display_dev == NULL) {
        printk("Device not found\n");
        return;
    }

    std::ostringstream myString1;
    myString1 << "Alpha: " << alphaPower;
    std::string alphaResult = myString1.str();

    std::ostringstream myString2;
    myString2 << "Beta: " << betaPpower;
    std::string betaResult = myString2.str();

    std::ostringstream myString3;
    myString3 << "Delta: " << deltaPower;
    std::string deltaResult = myString3.str();

    std::ostringstream myString4;
    myString4 << "Theta: " << thetaPower;
    std::string thetaResult = myString4.str();

    lv_obj_t *label1 = lv_label_create(lv_scr_act(), NULL); //ensure the active screen is being used for displaying
    lv_label_set_text(label, alphaResult); 
    lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 0, 0);  // coordinates for placing text
    lv_task_handler(); // continuously updates display based on changes in text...might need to be in a while loop?
    display_blanking_off(display_dev); //keeps the screen on (no blanking)

    //need to add a delay before displaying other text
    k_sleep(K_MSEC(4000));

    lv_obj_t *label1 = lv_label_create(lv_scr_act(), NULL); //ensure the active screen is being used for displaying
    lv_label_set_text(label, betaResult); 
    lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 0, 0);  // coordinates for placing text
    lv_task_handler(); // continuously updates display based on changes in text...might need to be in a while loop?
    display_blanking_off(display_dev); //keeps the screen on (no blanking)

    k_sleep(K_MSEC(4000));

    lv_obj_t *label1 = lv_label_create(lv_scr_act(), NULL); 
    lv_label_set_text(label, deltaResult); 
    lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 0, 0);  
    lv_task_handler(); 
    display_blanking_off(display_dev); 

    k_sleep(K_MSEC(4000));

    lv_obj_t *label1 = lv_label_create(lv_scr_act(), NULL); 
    lv_label_set_text(label, thetaResult); 
    lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 0, 0); 
    lv_task_handler(); 
    display_blanking_off(display_dev); 

    k_sleep(K_MSEC(4000));

}


void main(void){
    lv_init(); //initialize lvgl
    lv_obj_t *screen = lv_obj_create(NULL, NULL); //create lvgl screen object
    lv_scr_load(screen);             //makes the screen object the active screen on the display
    //void displayMessage(const char *message);
    void displayMessage();
    void displayResults();
}

// "Welcome"
// "Press to start recording EEG"
// "Recording in progress...press again to cancel"
// "ERROR"
// "You have cancelled the session"
// "Recording complete, analyzing data..."
// "Data Analysis Complete"
// "Your EEG is normal"
// "Your EEG may be abnormal"

/*

    
   
     
    //st7735r_get_capabilities(display_dev, &capabilities);
    //struct display_buffer_descriptor desc;
    //st7735r_get_framebuffer(display_dev, &desc);
    //write to display
    //st7735r_write(display_dev, x, y, &desc, message);
    

    
    static lv_disp_buf_t disp_buf;
    static lv_color_t buf[LV_HOR_RES_MAX * 10];
    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = display_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    lv_disp_buf_fill(&disp_buf, LV_COLOR_WHITE);

    
}
*/