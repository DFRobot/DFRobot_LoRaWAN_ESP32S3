/*!
 *@file ABP.ino
 *@brief Node joins the network via ABP and uses Confirmed packet communication with the gateway.
 *@details The node is activated via the ABP (Activation By Personalization) method. It transmits 
           application data every 10 seconds through a ​​Confirmed Uplink​​ packet and waits for the 
           gateway to return an ​​ACK​​ acknowledgment packet.Network joining status, data transmission, 
           ACK reception, and regular packet information will be printed via the serial port.
 *@copyright Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 *@licence The MIT License (MIT)
 *@author [Martin](Martin@dfrobot.com)
 *@version V0.0.1
 *@date 2025-2-27
 *@url https://github.com/DFRobot/DFRobot_LoRaWAN_ESP32S3
 */
#include "DFRobot_LoRaWAN.h"
#define APP_INTERVAL_MS 10000

// LoRaWAN DevAddr
const uint32_t nodeDevAddr = 0xDF666666;

// LoRaWAN NWKSKEY
const uint8_t nodeNwsKey[16] = {
0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
};

// LoRaWAN APPSKEY
const uint8_t nodeAppsKey[16] = {
0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10
};

// LoRaWAN application data buffer
uint8_t buffer[255];
// Application port number
uint8_t port = 2;

//LoRaWAN_Node node(/*devAddr=*/nodeDevAddr, /*NwkSKey = */nodeNwsKey, /*AppSKey=*/nodeAppsKey, /*classType=*/CLASS_A);
LoRaWAN_Node node(/*devAddr=*/nodeDevAddr, /*NwkSKey = */nodeNwsKey, /*AppSKey=*/nodeAppsKey);
TimerEvent_t appTimer;

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

void rxCb(void *buffer,uint16_t size,uint8_t port,int16_t rssi,int8_t snr,bool ackReceived,uint16_t uplinkCounter ,uint16_t downlinkCounter)
{
    printf("\nrecv packet\nuplinkCounter = %d\n", uplinkCounter);
    printf("downlinkCounter = %d\n", downlinkCounter);
    if(ackReceived == true){
        printf("this is a ACK rssi = %d snr = %d\n", rssi, snr);
    }

    if(size != 0){
        printf("port %d, size:%d, Rssi:%d, snr:%d, data:%s\n", port, size, rssi, snr, (uint8_t*)buffer);
        for(uint8_t i = 0; i < size; i++){ 
            printf("0x%x",((uint8_t*)buffer)[i]);
        }
        printf("\n");
    }
    printf("\n");
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
    printf("ABP Test\n");

    TimerSetValue(&appTimer, APP_INTERVAL_MS);
    TimerStart(&appTimer);                         // Start a timer to send data
}

void loop()
{
    delay(1000);
}