/*!
 *@file LoRaSend.ino
 *@brief Send a LoRa message over the air.
 *@details Set frequency, SF, and power, send data every 3 seconds, then print result on screen and serial.
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

DFRobot_LoRaRadio radio;
uint8_t buffer[4] = {1, 2, 3, 4};
uint32_t counter = 0;

// Transmission complete callback function
void loraTxDone(void)
{
    printf("-------------------------LoRa Tx done-----------------------------\n");
    screen.fillScreen(BG_COLOR);    
    screen.setTextColor(TEXT_COLOR);
    screen.setFont(TEXT_FONT);
    screen.setTextSize(TEXT_SIZE);
    screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_1);
    screen.printf("Send %dst", counter);
    screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_2);
    screen.printf("done");
}

void setup()
{
    Serial.begin(115200); // Initialize serial communication with a baud rate of 115200
    screen.begin();
    /*
    * The screen backlight is turned on by default. 
    * You can use this method to control the backlight switch. 
    * When the backlight is turned off, the screen will not display any content.
    */
    // screen.Backlight(ON);
    screen.fillScreen(BG_COLOR);    
    screen.setTextColor(TEXT_COLOR);
    screen.setFont(TEXT_FONT);
    screen.setTextSize(TEXT_SIZE);
    screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_1);
    screen.printf("LoRa Send Test");

    delay(5000);   // Open the serial port within 5 seconds after uploading to view full print output
    radio.init();  // Initialize the LoRa node with a default bandwidth of 125 KHz    
    radio.setTxCB(loraTxDone);                  // Set the transmission complete callback function
    radio.setFreq(RF_FREQUENCY);                // Set the communication frequency
    radio.setEIRP(TX_EIRP);                     // Set the Tx Eirp
    radio.setSF(LORA_SPREADING_FACTOR);         // Set the spreading factor
    radio.setBW(BW_125);                        // Set the bandwidth
}

void loop()
{
    printf("statistics: send %d packet\n", ++counter);  // Print the prompt message
    radio.sendData(buffer, /*size=*/4);                 // Send data
    delay(3 * 1000);                                    // Delay 3 seconds before sending next data
}