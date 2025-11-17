/*!
 *@file DeepSleepWakeUp.ino
 *@brief LoRaWAN low-power mode.
 *@details The node transmits data, enters deep sleep, and wakes periodically for the next transmission. 
           A button press during sleep forces wakeup to log network parameters.If the join fails, we 
           implement a backoff retry mechanism with intervals ranging from 5 seconds up to a maximum of 5 minutes.
 *@copyright Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 *@licence The MIT License (MIT)
 *@author [Martin](Martin@dfrobot.com)
 *@version V0.0.1
 *@date 2025-2-21
 *@url https://github.com/DFRobot/DFRobot_LoRaWAN_ESP32S3
 */

#include "DFRobot_LoRaWAN.h"

// Button pin
#define BTN_PIN 18  // GPIO2, 3, 11, 12, 13 can all trigger external wake-up

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
// LoRaWAN_Node node(DevEUI, AppEUI, AppKey, /*classType=*/CLASS_A);
LoRaWAN_Node node(DevEUI, AppEUI, AppKey);
// Rejoin count
RTC_DATA_ATTR uint8_t CurReJoinTimes = 0;
// ​Downlink Reception Success Flag​
uint8_t rxFlag = 1;
uint32_t prevTimeStamp = 0;

const char* wakeup_reason_strings[] = 
{
    "ESP_SLEEP_WAKEUP_UNDEFINED",
    "ESP_SLEEP_WAKEUP_ALL",
    "ESP_SLEEP_WAKEUP_EXT0",
    "ESP_SLEEP_WAKEUP_EXT1",
    "ESP_SLEEP_WAKEUP_TIMER",
    "ESP_SLEEP_WAKEUP_TOUCHPAD",
    "ESP_SLEEP_WAKEUP_ULP",
    "ESP_SLEEP_WAKEUP_GPIO",
    "ESP_SLEEP_WAKEUP_UART",
    "ESP_SLEEP_WAKEUP_WIFI",
    "ESP_SLEEP_WAKEUP_COCPU",
    "ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG",
    "ESP_SLEEP_WAKEUP_BT"
};

// Join network callback function
void joinCb(bool isOk, int16_t rssi, int8_t snr)
{
    screen.fillScreen(BG_COLOR);    
    screen.setTextColor(TEXT_COLOR);
    screen.setFont(TEXT_FONT);
    screen.setTextSize(TEXT_SIZE);

    if(isOk){
        CurReJoinTimes = 0;
        printf("JOIN SUCCESS\n");
        printf("JoinAccept Packet rssi = %d snr = %d\n", rssi, snr);
        printf("NetID = %06X\n", node.getNetID());
        printf("DevAddr = %08X\n", node.getDevAddr());
        uint8_t * NwkSKey = node.getNwkSKey();
        uint8_t * AppSKey = node.getAppSKey();
        printf("NwkSKey=0X");
        for(uint8_t i= 0; i < 16; i++){
            printf("%02X", NwkSKey[i]);
        }
        printf("\n");
        printf("AppSKey=0X");
        for(uint8_t i = 0; i < 16; i++){
            printf("%02X", AppSKey[i]);
        }
        printf("\n");

        screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_1);
        screen.printf("JOIN SUCCESS");
        screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_2);
        screen.printf("Accept Packet");
        screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_3);
        screen.printf("Rssi = %d", rssi);
        screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_4);
        screen.printf("Snr = %d", snr);
        delay(5000);

        node.deepSleepMs(APP_INTERVAL_MS);  // Deep sleep after successful network join        

    }else{
        printf("OTAA join error\n");
        printf("Check Whether the device has been registered on the gateway!\n");
        printf("deviceEUI and appKey are the same as the devices registered on the gateway\n");
        printf("Ensure that there is a gateway nearby\n");
        printf("Check whether the antenna is normal\n");
        screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_1);
        screen.printf("OTAA join Err!");
        delay(2000);    

        // Backoff join procedure
        CurReJoinTimes++; 
        // printf("\n\n------CurReJoinTimes = %d------\n\n", CurReJoinTimes);
        uint64_t backoff_time_ms = 5000 * (1ULL << (CurReJoinTimes - 1));
        backoff_time_ms = (backoff_time_ms > 300000) ? 300000 : backoff_time_ms;
        node.deepSleepMs(backoff_time_ms);
    }  
}

void userSendConfirmedPacket(void)
{    
    const char * data = "DFRobot"; 
    uint32_t datalen = strlen(data);
    memcpy(buffer, data, datalen);    
    node.sendConfirmedPacket(port, buffer, /*size=*/datalen);
    rxFlag = 0;
    
    screen.fillScreen(BG_COLOR);    
    screen.setTextColor(TEXT_COLOR);
    screen.setFont(TEXT_FONT);
    screen.setTextSize(TEXT_SIZE);
    screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_1);
    screen.printf("Sending...");
    screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_2);
    screen.printf("Confirmed");
    screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_3);
    screen.printf("Packet");
}

// Send data callback function
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

// Receive data callback function
void rxCb(void *buffer, uint16_t size, uint8_t port, int16_t rssi, int8_t snr, bool ackReceived, uint16_t uplinkCounter, uint16_t downlinkCounter)
{
    rxFlag = 1;
    screen.fillScreen(BG_COLOR);    
    screen.setTextColor(TEXT_COLOR);
    screen.setFont(TEXT_FONT);
    screen.setTextSize(TEXT_SIZE);
    if(ackReceived == true){
        screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_1);
        screen.printf("this is a ACK");
        screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_2);
        screen.printf("Rssi = %d", rssi);
        screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_3);
        screen.printf("Snr = %d", snr);
        screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_4);
        screen.printf("DownCount = %d", downlinkCounter);
    }
    delay(3000);

    // You can print the received data here
    // if(size != 0){
    //     printf("Data:%s\n", (uint8_t*)buffer);
    //     for(uint8_t i = 0; i < size; i++){
    //         printf(",0x%x",((uint8_t*)buffer)[i]);
    //     }
    // }
    node.deepSleepMs(APP_INTERVAL_MS);      // MCU sleep for a specified duration
}

// Handle button-triggered wakeup: display node info and return to sleep
void buttonWakeupHandler()
{    
    screen.fillScreen(BG_COLOR);    
    screen.setTextColor(TEXT_COLOR);
    screen.setFont(TEXT_FONT);
    screen.setTextSize(TEXT_SIZE);
    screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_1);
    screen.printf("buttonCB");
    screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_2);
    screen.printf("dataRate: %d\n", node.getDataRate());
    screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_3);
    screen.printf("txEirp: %d\n", node.getEIRP());
    screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_4);
    screen.printf("netID: %d\n", node.getNetID());
    printf("LastDownlinkCounter = %d\n", node.getLastDownCounter());
    printf("LastUplinkCounter = %d\n", node.getLastUplinkCounter());
    
    delay(5000);
    node.deepSleepMs(APP_INTERVAL_MS);      // MCU sleep for a specified duration
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
    screen.printf("WakeUp");
    delay(2000); 

    // Set to wake up using a button press
    esp_sleep_enable_ext0_wakeup((gpio_num_t )BTN_PIN, LOW);
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    if (wakeup_reason >= ESP_SLEEP_WAKEUP_UNDEFINED && wakeup_reason <= ESP_SLEEP_WAKEUP_BT) {
        printf("\n\n------Wakeup reason: [%s]------\n\n", wakeup_reason_strings[wakeup_reason]);
    } else {
        printf("\n\n------Wakeup reason: [UNKNOWN]------\n\n");
    }
    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
        buttonWakeupHandler();
    }

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

    node.setTxCB(txCb);                                 // Set the callback function for sending data
    node.setRxCB(rxCb);                                 // Set the callback function for receiving data

    if(!node.isJoined()) {
        screen.fillScreen(BG_COLOR);        
        screen.setTextColor(TEXT_COLOR);
        screen.setFont(TEXT_FONT);
        screen.setTextSize(TEXT_SIZE);
        screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_1);
        screen.printf("Join Request");
        node.join(joinCb);                                  // Join the LoRaWAN network 
    } else {
        userSendConfirmedPacket();                          // Send data
    }    
}

void loop()
{
    // ​​Prevent prolonged downlink waiting from blocking deep sleep and increasing power consumption​
    uint32_t currTimeStamp = TimerGetCurrentTime();
    if (currTimeStamp - prevTimeStamp >= APP_INTERVAL_MS * 2) {
        prevTimeStamp = currTimeStamp;    
        if (!rxFlag) {
            node.deepSleepMs(APP_INTERVAL_MS);
        } 
    }

    delay(1000);
}

