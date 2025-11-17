/*!
 *@file LoRaAddrSend.ino
 *@brief LoRa communication with devices at specified addresses.
 *@details In multi-node communication, include target address and source address information. 
           Each device can only parse messages with its own address as the target or messages 
           from a broadcast.
 *@copyright Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 *@licence The MIT License (MIT)
 *@author [Martin](Martin@dfrobot.com)
 *@version V0.0.1
 *@date 2025-3-12
 *@url https://github.com/DFRobot/DFRobot_LoRaWAN_ESP32S3
 */
#include "DFRobot_LoRaRadio.h"

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

char message[] = "Hello";
uint8_t buffer[36];
byte localAddr = 0xAA;        // Current device address
byte destAddr  = 0xBB;        // Target device address
byte msgCount = 0;            // Send message count
long prevTimeStamp = 0;       // Message timestamp
int interval = 10000;         // Send interval

// Transmission complete callback function
void loraTxDone(void)
{
    printf("-------------------------tran done-----------------------------\n");
}

void setup()
{
    Serial.begin(115200);   // Initialize serial communication with a baud rate of 115200
    delay(5000);
    printf("localAddr = 0x%02x\n", localAddr);
    printf("destAddr = 0x%02x\n", destAddr);
    
    radio.init();           // Initialize the LoRa node with a default bandwidth of 125 KHz
    radio.setTxCB(loraTxDone);                    // Set the transmission complete callback function
    radio.setFreq(RF_FREQUENCY);                   // Set the communication frequency
    radio.setEIRP(TX_EIRP);
    radio.setSF(LORA_SPREADING_FACTOR);
}

void loop()
{
    if(TimerGetCurrentTime() - prevTimeStamp > interval){
        buffer[0] = destAddr;
        buffer[1] = localAddr;
        buffer[2] = msgCount;
        buffer[3] = sizeof(message);
        memcpy(buffer+4, message, sizeof(message)+1);
        radio.sendData(buffer, sizeof(message)+4);
        printf("Sending %s to Addr 0x%x\n" ,message,destAddr);
        prevTimeStamp = TimerGetCurrentTime();            // Message timestamp
        msgCount++;
    }
}