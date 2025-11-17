/*!
 *@file OTAAConfirmedPacket.ino
 *@brief Node joins the network via OTAA and uses Confirmed packet communication with the gateway.
 *@details The node joins the network using the OTAA (Over-The-Air Activation) method. 
           After successfully joining the network, it sends application data via a Confirmed 
           Uplink packet every 10 seconds and waits for an ACK packet from the gateway. 
           Information such as network joining, data transmission, receiving ACK packets, 
           and regular data packets will be printed on the serial port.
 *@copyright Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 *@licence The MIT License (MIT)
 *@author [Martin](Martin@dfrobot.com)
 *@version V0.0.1
 *@date 2025-2-24
 *@url https://github.com/DFRobot/DFRobot_LoRaWAN_ESP32S3
 */
#include "DFRobot_LoRaWAN.h"

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
TimerEvent_t appTimer;

void joinCb(bool isOk, int16_t rssi, int8_t snr)
{
    if(isOk){
        printf("JOIN SUCCESS\n");
        printf("JoinAccept Packet rssi = %d snr = %d\n", rssi, snr);
        printf("NetID = %06X\n", node.getNetID());
        printf("DevAddr = %08X\n", node.getDevAddr());
        uint8_t * NwkSKey = node.getNwkSKey();
        uint8_t * AppSKey = node.getAppSKey();

        printf("NwkSKey = 0X");
        for(uint8_t i = 0; i < 16; i++){
            printf("%02X", NwkSKey[i]);
        }
        printf("\n");
        printf("AppSKey = 0X");
        for(uint8_t i = 0; i < 16; i++){
            printf("%02X", AppSKey[i]);
        }
        printf("\n");
        TimerSetValue(&appTimer, APP_INTERVAL_MS);
        TimerStart(&appTimer);
    }else{
        printf("OTAA join error\n");
        printf("Check Whether the device has been registered on the gateway!\n");
        printf("deviceEUI and appKey are the same as the devices registered on the gateway\n");
        printf("Ensure that there is a gateway nearby\n");
        printf("Check whether the antenna is normal\n");

        delay(5000);
        printf("Restart Join Request Packet\n");
        node.join(joinCb);      // Rejoin the LoRaWAN network        
    }
}

void userSendConfirmedPacket(void)
{
    TimerSetValue(&appTimer, APP_INTERVAL_MS);
    TimerStart(&appTimer);    

    const char * data = "DFRobot"; 
    uint32_t datalen = strlen(data);
    memcpy(buffer, data, datalen);
    node.sendConfirmedPacket(port, buffer, /*size=*/datalen);

    printf("Sending Confirmed Packet...\n");
}

// Send data callback function
void txCb(bool isconfirm, int8_t datarate, int8_t txeirp, uint8_t Channel)
{
    // Real transmission power and other information can be printed within the send data callback function
    printf(isconfirm?"Confirm Packet\n":"Unconfirm Pack\n");
    printf("datarate: %d\n", datarate);
    printf("txeirp: %d\n", txeirp);
}

// Receive data callback function
void rxCb(void *buffer, uint16_t size, uint8_t port, int16_t rssi, int8_t snr, bool ackReceived, uint16_t uplinkCounter, uint16_t downlinkCounter)
{
    if(ackReceived == true){
        printf("this is a ACK Rssi = %d Snr = %d\n", rssi, snr);
        printf("uplinkCounter = %d\n", uplinkCounter);
        printf("downlinkCounter = %d\n", downlinkCounter);
    }

    if(size != 0){
        printf("port %d, size:%d, Rssi:%d, snr:%d, data:%s\n", port, size, rssi, snr, (uint8_t*)buffer);
        for(uint8_t i = 0; i < size; i++){
            printf(",0x%X", ((uint8_t*)buffer)[i]);
        }
        printf("\n");
    }
}

void setup()
{
    Serial.begin(115200);
    delay(5000); // Open the serial port within 5 seconds after uploading to view full print output
    
    /*
    * Region | Data Rate | Tx Eirp (Even Numbers Only)
    * ------------------------------------------------
    *  EU868 | DR0 ~ DR5 | 2, 4, 6 ~ 16 dBm
    *  US915 | DR0 ~ DR4 | 2, 4, 6 ~ 22 dBm
    */     
    if(!(node.init(/*dataRate=*/DR_4, /*txEirp=*/16))){     // Initialize the LoRaWAN node, set the data rate and Tx Eirp
        printf("LoRaWAN Init Failed!\nPlease Check: DR or Region\n");
        while(1);
    }
    //node.init(DR_5, 16, /*adr = */false, /*dutyCycle =*/LORAWAN_DUTYCYCLE_OFF);
    TimerInit(&appTimer, userSendConfirmedPacket); // Initialize timer event
    node.setTxCB(txCb);                             // Set the callback function for sending data
    node.setRxCB(rxCb);                             // Set the callback function for receiving data
    node.join(joinCb);                                  // Join the LoRaWAN network
    printf("Join Request Packet\n");
}

void loop()
{
    delay(100);
}