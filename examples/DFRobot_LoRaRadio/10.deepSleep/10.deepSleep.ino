/*!
 *@file deepSleep.ino
 *@brief LoRa low-power communication.
 *@details After sending the LoRa message, enter deep sleep and set a timer to
           wake up after 10 seconds or upon button press.
 *@n During sleep mode, the device must enter ​​boot mode​​ to flash new firmware.
 *@copyright Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 *@licence The MIT License (MIT)
 *@author [Martin](Martin@dfrobot.com)
 *@version V0.0.1
 *@date 2025-3-12
 *@url https://github.com/DFRobot/DFRobot_LoRaWAN_ESP32S3
 */
#include "DFRobot_LoRaRadio.h"

LCD_OnBoard screen;

#define BG_COLOR        COLOR_RGB565_BLACK      // Screen background color
#define TEXT_COLOR      COLOR_RGB565_GREEN      // Screen font color
/*
* Recommended font
* -------------------------------------------------------------------------------
* FreeMono9pt7b, FreeMono12pt7b, FreeMonoBold9pt7b, FreeSans9pt7b, FreeSerif9pt7b
* For more fonts, see the directory: src\external\DFRobot_GDL_LW\src\Frame\Fonts\
*/ 
#define TEXT_FONT       &FreeMono9pt7b          // font
#define TEXT_SIZE       1                       // Screen font size
#define LINE_HEIGHT     18                      // Line height
#define POX_X           0                       // Screen print position X coordinate
#define POX_Y           15                      // Screen print position Y coordinate
#define LINE_1          0                       // Line number
#define LINE_2          1
#define LINE_3          2
#define LINE_4          3

/*
* Region | Spreading Factor | Tx Eirp (Even Numbers Only)
* -------------------------------------------------------
*  EU868 |    SF7 ~ SF12    | 2, 4, 6 ~ 16 dBm
*  US915 |    SF8 ~ SF12    | 2, 4, 6 ~ 22 dBm
*/  
#ifdef REGION_EU868
#define RF_FREQUENCY 868000000  // Hz
#define TX_EIRP 16    // dBm 
#endif
#ifdef REGION_US915
#define RF_FREQUENCY 915000000  // Hz
#define TX_EIRP 22    // dBm 
#endif
#define LORA_SPREADING_FACTOR 7

// Button pin
#define BTN_PIN 18  // GPIO2, 3, 11, 12, 13 can all trigger external wake-up

DFRobot_LoRaRadio radio;
RTC_DATA_ATTR uint32_t counter = 0;
uint8_t buffer[4];

// Transmission complete callback function
void loraTxDone(void)
{
    printf("-------------------------Send done-----------------------------\n");

    screen.fillScreen(BG_COLOR);    
    screen.setTextColor(TEXT_COLOR);
    screen.setFont(TEXT_FONT);
    screen.setTextSize(TEXT_SIZE);
    screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_1);
    screen.printf("Send done");
    screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_3);
    screen.printf("Enter Sleep");
    delay(1000);
    // Enter deep sleep 10s
    radio.deepSleepMs(10000);
}

void setup()
{
    Serial.begin(115200);   // Initialize serial communication with a baud rate of 115200
    radio.init();           // Initialize the LoRa node with a default bandwidth of 125 KHz
    screen.begin();
    /*
    * The screen backlight is turned on by default. 
    * You can use this method to control the backlight switch. 
    * When the backlight is turned off, the screen will not display any content.
    */
    // screen.Backlight(ON);

    radio.setTxCB(loraTxDone);                    // Set the transmission complete callback function
    radio.setFreq(RF_FREQUENCY);                  // Set the communication frequency
    radio.setEIRP(TX_EIRP);                       // Set the Tx Eirp
    radio.setSF(LORA_SPREADING_FACTOR);           // Set the spreading factor

    // Set to wake up using a button press
    esp_sleep_enable_ext0_wakeup((gpio_num_t )BTN_PIN, LOW); 

    counter++;
    screen.fillScreen(BG_COLOR);    
    screen.setTextColor(TEXT_COLOR);
    screen.setFont(TEXT_FONT);
    screen.setTextSize(TEXT_SIZE);
    screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_1);
    screen.printf("WakeUp");
    screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_3);
    screen.printf("Send %dst", counter);
    printf("Send the %d times\n", counter);
    delay(1000);

    buffer[0] = (counter >> 24) & 0xff;
    buffer[1] = (counter >> 16) & 0xff;
    buffer[2] = (counter >> 8) & 0xff;
    buffer[3] = counter & 0xff;

    radio.sendData(buffer, 4); // Send data

}

void loop()
{
    delay(10000);
}
