/*!
 *@file DFRobot_LoRaRadio.h
 *@brief Define the basic structure of class DFRobot_LoRaRadio, the implementation of basic methods.
 *@copyright Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 *@licence The MIT License (MIT)
 *@author [Martin](Martin@dfrobot.com)
 *@version V0.0.1
 *@date 2025-3-14
 *@get from https://www.dfrobot.com
 *@url https://github.com/DFRobot/DFRobot_LoRaWAN_ESP32S3
*/
#ifndef __DFROBOT_LORARADIO_H_
#define __DFROBOT_LORARADIO_H_

#include "external\DFRobot_GDL_LW\src\DFRobot_GDL_LW.h"
#include "radio/radio.h"
#include <string.h>
#include "radio/sx126x/sx126x.h"
#include "boards/mcu/timer.h"

#define LCD_OnBoard LoRaWAN::DFRobot_ST7735_80x160_HW_SPI ///< The type of screen on the development board
#define SPI_MUTEX LoRaWAN::spimutex


/**
 * @enum eBandwidths_t
 * @brief LoRa communication uses specific bandwidths.
 * @details When the bandwidth is larger, the data transmission rate increases, 
 *          while a smaller bandwidth provides stronger anti-interference capabilities 
 *          and enables longer transmission distances.
 */
typedef enum
{
      BW_125 = 0,   /**<125k HZ>*/
      BW_250,       /**<250k HZ>*/
      BW_500,       /**<500k HZ>*/
      BW_062,       /**<62k HZ>*/
      BW_041,       /**<41k HZ>*/
      BW_031,       /**<31k HZ>*/
      BW_020,       /**<20k HZ>*/
      BW_015,       /**<15k HZ>*/
      BW_010,       /**<10k HZ>*/
      BW_007 = 9,   /**<7k HZ>*/
} eBandwidths_t;

/**
 * @fn txCB
 * @brief Callback function for when data transmission is completed.
 */
typedef void txCB(void);

/**
 * @fn rxCB
 * @brief Callback function for when data reception is completed.
 * @param payload The received payload.
 * @param size The size of the received payload.
 * @param rssi The received signal strength indicator.
 * @param snr The signal-to-noise ratio.
 */
typedef void rxCB(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);

/**
 * @fn cadDoneCB
 * @brief Callback function for when channel activity detection is completed.
 * @param cadResult Whether the channel is free or busy.
 */
typedef void cadDoneCB(bool cadResult);

/**
 * @fn rxErrorCB
 * @brief Callback function for when data reception encounters an error.
 */
typedef void rxErrorCB(void);

/**
 * @brief Class for interfacing with a LoRa radio module using the Semtech SX126x chip.
 */
class DFRobot_LoRaRadio
{

public:

      /**
       * @fn DFRobot_LoRaRadio
       * @brief Constructor for the DFRobot_LoRaRadio class.
       * @return LoRaRadio node object
       */
      DFRobot_LoRaRadio();

      /**
       * @fn init
       * @brief Initializes the LoRa radio module.
       * @return None
       */
      void init();

      /**
       * @fn setEIRP
       * @brief Sets the transmission power of the LoRa radio module.
       * @param EIRP Equivalent Isotropically Radiated Power(dBm)
       * @return None
       */
      void setEIRP(int8_t EIRP);

      /**
       * @fn setSF
       * @brief Set the spreading factor of the radio.
       * @param SF The spreading factor to set (unsigned 8-bit integer).
       * @return None
       */
      void setSF(uint8_t SF);

      /**
       * @fn setBW
       * @brief Set the bandwidth of the radio.
       * @param BW The bandwidth to set (enumeration type eBandwidths_t).
       * @return None
       */
      void setBW(eBandwidths_t BW);

      /**
       * @fn setFreq
       * @brief Sets the frequency of the LoRa radio module.
       * @param freq The frequency, in Hz.
       * @return None
       */
      void setFreq(uint32_t freq);

      /**
       * @fn setTxCB
       * @brief Sets the callback function for when data transmission is completed.
       * @param cb The callback function.
       * @return None
       */
      void setTxCB(txCB cb);

      /**
       * @fn sendData
       * @brief Sends data using the LoRa radio module.
       * @param data Pointer to the data to be sent.
       * @param size The length of the data, in bytes.
       * @return None
       */
      void sendData(const void *data, uint8_t size);

      /**
       * @fn setRxCB
       * @brief Sets the callback function for when data reception is completed.
       * @param cb The callback function.
       * @return None
       */
      void setRxCB(rxCB cb);

      /**
       * @fn setRxErrorCB
       * @brief Sets the callback function for when data reception encounters an error.
       * @param cb The callback function.
       * @return None
       */
      void setRxErrorCB(rxErrorCB cb);

      /**
       * @fn startRx
       * @brief Starts receiving data using the LoRa radio module.
       * @return None
       */
      void startRx();

      /**
       * @fn stopRx
       * @brief LoRa radio module stops receiving data.
       * @return None
       */
      void stopRx();

      /**
       * @fn setCadCB
       * @brief Sets the callback function for when channel activity detection is completed.
       * @param cb The callback function.
       * @return None
       */
      void setCadCB(cadDoneCB cb);

      /**
       * @fn startCad
       * @brief Starts channel activity detection using the LoRa radio module.
       * @param cadSymbolNum the number of symbols to be used for channel activity detection operation.
       * @param cadDetPeak Peak detection threshold, Signals above this threshold are considered definite channel activity.
       * @param cadDetMin Minimum detection threshold, Signals between cadDetMin and cadDetPeak trigger potential activity.
       * @return None
       */
      void startCad(RadioLoRaCadSymbols_t cadSymbolNum, uint8_t cadDetPeak, uint8_t cadDetMin);

      /**
       * @fn deepSleepMs
       * @brief Set the MCU to immediately enter sleep for a specified duration.
       * @param timesleep Node sleep duration(ms).If set to 0, the device will never wake up.
       * @return None
       */
      void deepSleepMs(uint32_t timesleep);

      /**
       * @fn setEncryptKey
       * @brief Set the encryption key for the radio.
       * @param key Pointer to the encryption key (unsigned 8-bit integer array).
       * @return None
       */
      void setEncryptKey(const uint8_t *key);

      /**
       * @fn dumpRegisters
       * @brief Dump the registers of the radio.
       * @n This function prints the current values of all the registers of the radio to the output (serial monitor)
       * @return None
       */
      void dumpRegisters();

      /**
       * @fn setSync
       * @brief This method is not available.
       * @return None
       */
      void setSync(uint16_t sync);

      private:
      eBandwidths_t _bandwidth = BW_125; /**< The bandwidth of the LoRa radio module. */
      uint8_t _txeirp = 16;
      uint8_t _SF = 7;
      
      protected:
     };
     
     #endif // DFRobot_LoraRadio_h