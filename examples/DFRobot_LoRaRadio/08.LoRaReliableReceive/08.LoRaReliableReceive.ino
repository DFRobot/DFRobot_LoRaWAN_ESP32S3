/*!
 *@file LoRaReliableReceive.ino
 *@brief LoRa reliable communication, receiver.
 *@details The node enters receiving mode. If a message is received, it performs the following checks, 
           sends an ACK confirmation, and then re-enters receiving mode.
 *@n If the current frame count differs from the previous frame count by more than 1, it indicates that the receiver has experienced data loss.
 *@n If the current frame count is less than the previous frame count, it suggests that the sender may have restarted.
 *@n If the current frame count is equal to the previous frame count, it indicates that the sender has lost the ACK.
 *@copyright Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 *@licence The MIT License (MIT)
 *@author [Martin](Martin@dfrobot.com)
 *@version V0.0.1
 *@date 2025-3-14
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

char *buffer = "ack";
int16_t prevCount = 0;    // The previous frame count
int16_t currCount = 0;    // The current frame count

void loraRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
    printf("LoRa data received on channel %ld, SF=%d Rssi=%d Snr=%d \n", RF_FREQUENCY, LORA_SPREADING_FACTOR, rssi, snr);
    currCount = (payload[0] << 8) | payload[1];
    printf("receive Count=%d\n", currCount);

    if((currCount - prevCount) > 1){
      printf("data loss (currCount=%d,prevCount=%d)\n",currCount,prevCount);
    }else if(prevCount > currCount){
      printf("Host restart occurs\n");
    }else if(prevCount == currCount){
      printf("The host currCount data. Procedure\n The previous ack was lost\n");
    }
    prevCount = currCount;
    radio.stopRx();
    radio.sendData(buffer, sizeof(buffer));
    delay(1000);
    radio.startRx();                        // Start receiving    
}

void setup()
{
    Serial.begin(115200);                   // Initialize serial communication with a baud rate of 115200
    delay(5000);                            // Open the serial port within 5 seconds after uploading to view full print output
    radio.init();                           // Initialize the LoRa node with a default bandwidth of 125 KHz
    
    radio.setRxCB(loraRxDone);              // Set the receive complete callback function
    radio.setFreq(RF_FREQUENCY);            // Set the communication frequency
    radio.setSF(LORA_SPREADING_FACTOR);     // Set the spreading factor
    radio.startRx();                        // Start receiving
}

void loop()
{
    delay(3000);
}