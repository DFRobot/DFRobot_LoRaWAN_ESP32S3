/*!
 *@file LoRaReliableSend.ino
 *@brief LoRa reliable communication, sender.
 *@details Specify the frequency, spreading factor, and transmit power 
           to send a LoRa message over the air. After successful transmission, 
           wait up to 5 seconds to receive an ACK from the receiver. If the 
           ACK is received, proceed to send the next frame of data. If no ACK 
           is received, retransmit the previous frame. If no ACK is received 
           for five consecutive attempts, print "Transmission error".
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

uint8_t buffer[2] = {0, 1};
char *ack = "ack";
uint16_t sendCount = 0;
uint8_t noAck = 0;

// Transmission complete callback function
void loraTxDone(void)
{
    printf("-------------------------LoRa Tx done-----------------------------\n");
    radio.startRx();    // Start receiving
    noAck++;
    if(noAck > 5) printf("Transmission error\n");
}

// Receive complete callback function
void loraRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
    printf("LoRa packet received on channel %ld, SF=%d Rssi=%d Snr=%d\n", RF_FREQUENCY, LORA_SPREADING_FACTOR, rssi, snr);
    if(strcmp(ack, (char*)payload) == 0){
        sendCount++;
        noAck = 0;
        printf("receive ack packet\n");
        buffer[0] = sendCount >> 8;
        buffer[1] = sendCount & 0xff;
    }else{
        uint8_t i = 0;
        printf("reveive error packet\n Packet={");
        for(; i < size-1; i++ ){
            printf("0x%02x, ", payload[i]);
        }
        printf("0x%02x}\n\n", payload[i]);
    }
}

void setup()
{
    Serial.begin(115200);               // Initialize serial communication with a baud rate of 115200
    delay(5000);                        // Open the serial port within 5 seconds after uploading to view full print output
    radio.init();                       // Initialize the LoRa node with a default bandwidth of 125 KHz
    radio.setRxCB(loraRxDone);          // Set the receive complete callback function
    radio.setTxCB(loraTxDone);          // Set the transmission complete callback function
    radio.setFreq(RF_FREQUENCY);        // Set the communication frequency
    radio.setEIRP(TX_EIRP);             // Set the Tx Eirp
    radio.setSF(LORA_SPREADING_FACTOR); // Set the spreading factor
}

void loop()
{
    delay(5000);
    radio.stopRx();
    radio.sendData(buffer, sizeof(buffer));
    printf("send Counter=%d\n",(buffer[0]<<8)|buffer[1]);
}