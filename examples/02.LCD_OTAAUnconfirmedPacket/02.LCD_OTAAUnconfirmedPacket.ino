/*!
 *@file LCD_OTAAUnconfirmedPacket.ino
 *@brief Node joins the network via OTAA and uses Unconfirmed packet communication with the gateway.
 *@details The node joins the network using the OTAA (Over-The-Air Activation) method. 
           After successfully joining the network, it sends application data via an Unconfirmed Uplink 
           packet every 10 seconds without waiting for an ACK packet from the gateway. Events such as 
           network joining, data transmission, and receiving regular data packets will be printed on 
           the screen.
 *@copyright Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 *@licence The MIT License (MIT)
 *@author [Martin](Martin@dfrobot.com)
 *@version V0.0.1
 *@date 2025-2-24
 *@url https://github.com/DFRobot/DFRobot_LoRaWAN_ESP32S3
 */

#include "DFRobot_LoRaWAN.h"

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

// Data packet transmission interval
#define APP_INTERVAL_MS 10000
// LoRaWAN DevEUI
const uint8_t DevEUI[8] = {0xDF, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
// LoRaWAN AppEUI/JoinEUI
const uint8_t AppEUI[8] = {0xDF, 0xB7, 0xB7, 0xB7, 0xB7, 0x00, 0x00, 0x00};
// LoRaWAN AppKEY
const uint8_t AppKey[16] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};

// LoRaWAN application data buffer
uint8_t buffer[255];
// Application port number
uint8_t port = 2;
// UnConfirmed packet count
uint32_t counter = 0;

// LoRaWAN_Node node(DevEUI, AppEUI, AppKey, /*classType=*/CLASS_A);
LoRaWAN_Node node(DevEUI, AppEUI, AppKey);
TimerEvent_t appTimer;

void joinCb(bool isOk, int16_t rssi, int8_t snr)
{
    screen.fillScreen(BG_COLOR);    
    screen.setTextColor(TEXT_COLOR);
    screen.setFont(TEXT_FONT);
    screen.setTextSize(TEXT_SIZE);

    if(isOk){
        screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_1);
        screen.printf("JOIN SUCCESS");
        screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_2);
        screen.printf("Accept Packet");
        screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_3);
        screen.printf("Rssi = %d", rssi);
        screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_4);
        screen.printf("Snr = %d", snr);

        TimerSetValue(&appTimer, APP_INTERVAL_MS);
        TimerStart(&appTimer);
    }else{
        screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_1);
        screen.printf("OTAA join Err!");

        delay(5000);
        screen.fillScreen(BG_COLOR);    
        screen.setTextColor(TEXT_COLOR);
        screen.setFont(TEXT_FONT);
        screen.setTextSize(TEXT_SIZE);
        screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_1);
        screen.printf("Restart");
        screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_2);
        screen.printf("Join Request");
        screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_3);
        screen.printf("Packet");
        node.join(joinCb);           // Rejoin the LoRaWAN network
    }
}

void userSendUnConfirmedPacket(void)
{
    TimerSetValue(&appTimer, APP_INTERVAL_MS);
    TimerStart(&appTimer);

    const char * data = "DFRobot"; 
    uint32_t datalen = strlen(data);
    memcpy(buffer, data, datalen);
    node.sendUnconfirmedPacket(port, buffer, /*size=*/datalen);

    screen.fillScreen(BG_COLOR);    
    screen.setTextColor(TEXT_COLOR);
    screen.setFont(TEXT_FONT);
    screen.setTextSize(TEXT_SIZE);
    screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_1);
    screen.printf("Sending %dst", counter++);
    screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_2);
    screen.printf("UnConfirmed");
    screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_3);
    screen.printf("packet ...");
}

void txCb(bool isconfirm, int8_t datarate, int8_t txeirp, uint8_t Channel)
{
    // Real transmission power and other information can be printed within the send data callback function
    // screen.fillScreen(BG_COLOR);    
    // screen.setTextColor(TEXT_COLOR);
    // screen.setFont(TEXT_FONT);
    // screen.setTextSize(TEXT_SIZE);

    // screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_1);
    // screen.printf(isconfirm?"Confirm Packet":"Unconfirm Pack");
    // screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_2);
    // screen.printf("datarate: %d", datarate);
    // screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_3);
    // screen.printf("txeirp: %d", txeirp);
    // screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_4);
    // screen.printf("Channel: %d", Channel);
}

void rxCb(void *buffer, uint16_t size, uint8_t port, int16_t rssi, int8_t snr, bool ackReceived, uint16_t uplinkCounter, uint16_t downlinkCounter)
{
    screen.fillScreen(BG_COLOR);    
    screen.setTextColor(TEXT_COLOR);
    screen.setFont(TEXT_FONT);
    screen.setTextSize(TEXT_SIZE);

    screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_1);
    screen.printf("Rssi = %d", rssi);
    screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_2);
    screen.printf("Snr = %d", snr);
    screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_3);
    screen.printf("UpCount = %d", uplinkCounter);
    screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_4);
    screen.printf("DownCount = %d", downlinkCounter);

    // You can print the received data here
    // if(size != 0){
    //     screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_1);
    //     screen.printf("Data:%s\n", (uint8_t*)buffer);
    //     screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_2);
    //     for(uint8_t i = 0; i < size; i++){
    //         screen.printf(",0x%x",((uint8_t*)buffer)[i]);
    //     }
    // }
}

void setup()
{
    Serial.begin(115200);
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
    screen.printf("LoRaWAN Node");
    screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_3);
    screen.printf("Unconfirmed");
    screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_4);
    screen.printf("Packet");
    delay(2000);
    
    /*
    * Region | Data Rate | Tx Eirp (Even Numbers Only)
    * ------------------------------------------------
    *  EU868 | DR0 ~ DR5 | 2, 4, 6 ~ 16 dBm
    *  US915 | DR0 ~ DR4 | 2, 4, 6 ~ 22 dBm
    */     
    if(!(node.init(/*dataRate=*/DR_4, /*txEirp=*/16))){     // Initialize the LoRaWAN node, set the data rate and Tx Eirp
        screen.fillScreen(BG_COLOR);        
        screen.setTextColor(TEXT_COLOR);
        screen.setFont(TEXT_FONT);
        screen.setTextSize(TEXT_SIZE);
        screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_1);
        screen.printf("LoRaWAN Init");
        screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_2);
        screen.printf("Failed!");
        screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_3);
        screen.printf("Please Check:");
        screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_4);
        screen.printf("DR or Region");
        while(1);
    }
    //node.init(DR_5, 16, /*adr = */false, /*dutyCycle =*/LORAWAN_DUTYCYCLE_OFF);
    TimerInit(&appTimer, userSendUnConfirmedPacket);   // Initialize timer event
    node.setTxCB(txCb);                                 // Set the callback function for sending data
    node.setRxCB(rxCb);                                 // Set the callback function for receiving data
    node.join(joinCb);                                      // Join the LoRaWAN network

    screen.fillScreen(BG_COLOR);
    screen.setTextColor(TEXT_COLOR);
    screen.setFont(TEXT_FONT);
    screen.setTextSize(TEXT_SIZE);
    screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_1);
    screen.printf("Join Request");
    screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_2);
    screen.printf("Packet");
}

void loop()
{
    delay(100);
}