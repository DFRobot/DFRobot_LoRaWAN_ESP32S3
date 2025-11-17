/*!
 *@file LoRaEncryptSend.ino
 *@brief Send an encrypted LoRa message over the air.
 *@details Set frequency, SF, power, and encryption key, send encrypted LoRa message every 10 seconds.
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
const uint8_t encryptKey[16] = {0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,
                                0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12};
uint8_t buffer[4] = {1, 2, 3, 4};
static uint32_t counter = 1;

// Transmission complete callback function
void loraTxDone(void)
{
    counter++;
    printf("-------------------------LoRa Tx done-----------------------------\n");
}

void setup()
{
    Serial.begin(115200);   // Initialize serial communication with a baud rate of 115200
    delay(5000);            // Open the serial port within 5 seconds after uploading to view full print output
    radio.init();           // Initialize the LoRa node with a default bandwidth of 125 KHz 
    radio.setEncryptKey(encryptKey);                    // Set the communication key
    radio.setTxCB(loraTxDone);                    // Set the transmission complete callback function
    radio.setFreq(RF_FREQUENCY);                   // Set the communication frequency
    radio.setEIRP(TX_EIRP);                           // Set the Tx Eirp
    radio.setSF(LORA_SPREADING_FACTOR);    // Set the spreading factor
}

void loop()
{    
    printf("statistics: send %d Encrypt packet\n", counter); // Print the prompt message

    radio.sendData(buffer, /*size=*/4); // Send data
    delay(10 * 1000);                   // Delay 10 seconds before sending next data
}