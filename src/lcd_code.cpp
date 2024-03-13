#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#include <iostream>  // Include standard C++ libraries
#include <thread>    // Include C++ threading library for delays

// Define pin numbers if needed
#define BUTTON_UP_PIN  2

#define TFT_CS     AK10
#define TFT_RST    B14
#define TFT_DC     B16
#define TFT_MOSI   AK8
#define TFT_MISO   AK12
#define TFT_SCLK   AL13

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
// color definitions
/*
const uint16_t  Display_Color_Black        = 0x0000;
const uint16_t  Display_Color_Blue         = 0x001F;
const uint16_t  Display_Color_Red          = 0xF800;
const uint16_t  Display_Color_Green        = 0x07E0;
const uint16_t  Display_Color_Cyan         = 0x07FF;
const uint16_t  Display_Color_Magenta      = 0xF81F;
const uint16_t  Display_Color_Yellow       = 0xFFE0;
const uint16_t  Display_Color_White        = 0xFFFF;
*/

// The colors we actually want to use
//uint16_t        Display_Text_Color         = Display_Color_Black;
//uint16_t        Display_Backround_Color    = Display_Color_White;

// assume the display is off until configured in setup()

//bool            isDisplayVisible        = false;


int main() {
    setup();
    while (true) {
        loop();
    }
    return 0;
}

void setup() {
    // Initialization code
    //Serial.begin(9600);
    pinMode(CS_PIN, OUTPUT);    // Chip select pin
    pinMode(RST_PIN, OUTPUT);   // Reset pin
    pinMode(DC_PIN, OUTPUT);    // Data/Command pin
    pinMode(MOSI_PIN, OUTPUT);  // MOSI pin
    pinMode(MISO_PIN, OUTPUT);  // MISO pin
    pinMode(SCLK_PIN, OUTPUT);  // Clock pin 

    //use this initializer if using a 0.96" 180x60 TFT:
    tft.initR(INITR_MINI160x80);  // Init ST7735S mini display

    // initialise the display
    tft.setFont();
    tft.fillScreen(ST7735_WHITE ST77XX_WHITE);
    tft.setTextColor(ST7735_BLACK ST77XX_BLACK);
    tft.setTextSize(1);

    // the display is now on
    //isDisplayVisible = true;

    //Welcome Screen
    tft.setCursor(10, 10); // Set cursor position
    tft.println("Welcome");
    // Wait for a delay
    std::this_thread::sleep_for(std::chrono::seconds(3)); // Delay for 3 seconds

    // Display menu selection
    tft.fillScreen(ST7735_WHITE ST77XX_WHITE);
    tft.println("Press to start recording EEG");
   


}

void loop() {
    //user starts recording
    /*
    if (digitalRead() == LOW){
        tft.println("Recording in progress, press again to cancel...");
    }
    // user cancels recording
    if (digitalRead() == HIGH){
        tft.println("You have terminated the recording session");
    }
    if (digitalRead() == LOW){
        tft.println("Error: ");
    }
    */
   /*
    if (button1 == 1){
        tft.println("Recording in progress, press again to cancel...");
    }
    // user cancels recording
    if (button1 == 2){
        tft.println("You have terminated the recording session");
    }
    if (button1 == 3){
        tft.println("Error: ");
    }
        tft.println("Recording Complete");
        std::this_thread::sleep_for(std::chrono::seconds(3)); // Delay for 3 seconds
        tft.fillScreen(Display_Backround_Color);
        // if results are normal
            tft.fillScreen(ST7735_GREEN ST77XX_GREEN);
            tft.println("Your EEG is normal");
        // if results are abnormal
            tft.fillScreen(ST7735_YELLOW ST77XX_YELLOW);
            tft.println("Your EEG may be abnormal...");

    */
   tft.println("Testing Display");

}





