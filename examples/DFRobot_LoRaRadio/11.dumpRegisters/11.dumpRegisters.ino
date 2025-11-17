/*!
 *@file dumpRegister.ino
 *@brief Print out all register values of the SX1262. Users can perform in-depth debugging based on these register values.
 *@copyright Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 *@licence The MIT License (MIT)
 *@author [Martin](Martin@dfrobot.com)
 *@version V0.0.1
 *@date 2025-3-12
 *@url https://github.com/DFRobot/DFRobot_LoRaWAN_ESP32S3
 */
#include "DFRobot_LoRaRadio.h"

DFRobot_LoRaRadio radio;

void setup()
{
    Serial.begin(115200);         // Initialize serial communication with a baud rate of 115200
    delay(5000);                  // Open the serial port within 5 seconds after uploading to view full print output
    radio.init();
    radio.dumpRegisters();
}

void loop()
{
    delay(3000);
}