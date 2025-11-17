/*!
 *@file LoRaCad.ino
 *@brief LoRa channel activity detection.
 *@details In LoRa communication, the specified frequency, spreading factor,
           and bandwidth constitute a channel. This example uses the "startCad"
           function to detect whether the channel is occupied. The detection 
           result is returned through the callback function "loraCadDone".
           Within "loraCadDone", "startCad" is called again to enable continuous
           channel activity detection.
 *@copyright Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 *@licence The MIT License (MIT)
 *@author [Martin](Martin@dfrobot.com)
 *@version V0.0.1
 *@date 2025-3-12
 *@url https://github.com/DFRobot/DFRobot_LoRaWAN_ESP32S3
 */
#include "DFRobot_LoRaRadio.h"

DFRobot_LoRaRadio radio;

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

void loraCadDone(bool busy)
{
    if(busy){        
        printf("channel busy, can NOT send data\n");
    }else{
        radio.sendData("hello", /*size=*/5);
        printf("channel free, sending 5 bytes data\n");
    }
    radio.startCad(LORA_CAD_02_SYMBOL, 22, 10);
}

// Transmission complete callback function
void loraTxDone(void)
{
    printf("-------------------------tran done-----------------------------\n");
}

void setup()
{
    Serial.begin(115200);
    delay(5000);                        // Open the serial port within 5 seconds after uploading to view full print output
    radio.init();
    radio.setCadCB(loraCadDone);
    radio.setTxCB(loraTxDone);          // Set the transmission complete callback function
    radio.setFreq(RF_FREQUENCY);        // Set the communication frequency
    radio.setEIRP(TX_EIRP);
    radio.setSF(LORA_SPREADING_FACTOR);
    radio.setBW(BW_125);                        // Set the bandwidth
    radio.setTxCB(loraTxDone);

    /**
     * BW125KHZ best settings.
     * ----------------------------------------------------------
     * | SF |          Symbols         | cadDetPeak | cadDetMin | 
     * |----|--------------------------|------------|-----------|
     * | 7  |    LORA_CAD_02_SYMBOL    |     22     |    10     | 
     * | 8  |    LORA_CAD_02_SYMBOL    |     22     |    10     | 
     * | 9  |    LORA_CAD_04_SYMBOL    |     23     |    10     | 
     * | 10 |    LORA_CAD_04_SYMBOL    |     24     |    10     | 
     * | 11 |    LORA_CAD_04_SYMBOL    |     25     |    10     |
     * | 12 |    LORA_CAD_04_SYMBOL    |     28     |    10     |
     * ----------------------------------------------------------
     * 
     * BW500KHZ best settings.
     * ----------------------------------------------------------
     * | SF |          Symbols         | cadDetPeak | cadDetMin | 
     * |----|--------------------------|------------|-----------|
     * | 7  |    LORA_CAD_04_SYMBOL    |     21     |    10     | 
     * | 8  |    LORA_CAD_04_SYMBOL    |     22     |    10     | 
     * | 9  |    LORA_CAD_04_SYMBOL    |     22     |    10     | 
     * | 10 |    LORA_CAD_04_SYMBOL    |     23     |    10     | 
     * | 11 |    LORA_CAD_04_SYMBOL    |     25     |    10     |
     * | 12 |    LORA_CAD_08_SYMBOL    |     29     |    10     |
     * ----------------------------------------------------------
     * 
     * For details, refer to: DFRobot_LoRaWAN\datasheets\SX126X CAD performance evaluation V2.1.pdf
     */
    radio.startCad(LORA_CAD_02_SYMBOL, 22, 10);
}

void loop()
{
    delay(10 * 1000);
}