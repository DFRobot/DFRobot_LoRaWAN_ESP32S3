/*!
 *@file DFRobot_LoRaWAN.h
 *@brief Define the basic structure of class LoRaWAN_Node, the implementation of basic methods.
 *@copyright Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 *@licence The MIT License (MIT)
 *@author [Martin](Martin@dfrobot.com)
 *@version V0.0.1
 *@date 2025-1-17
 *@get from https://www.dfrobot.com
 *@url https://github.com/DFRobot/DFRobot_LoRaWAN_ESP32S3
 */
#ifndef __LORAWANNODE_H_
#define __LORAWANNODE_H_

#include "external\DFRobot_GDL_LW\src\DFRobot_GDL_LW.h"
#include "boards/mcu/board.h"
#include "system/utilities.h"
#include "boards/mcu/timer.h"
#include "radio/radio.h"
#include "mac/LoRaMacTest.h"
#include "stdint.h"
#include "mac/Commissioning.h"
#include "mac/LoRaMac.h"
#include "mac/region/Region.h"

#define LCD_OnBoard LoRaWAN::DFRobot_ST7735_80x160_HW_SPI ///< The type of screen on the development board
#define SPI_MUTEX LoRaWAN::spimutex

/**
 * @fn joinCallback
 * @brief Callback function for when data transmission is completed.
 * @details The user-defined network connection callback function can obtain the network connection result, 
 *          the signal strength of the gateway's accept packet, and the signal-to-noise ratio.
 * @param isOk Whether the network connection is successful
 * @param rssi Received Signal Strength Indication(dBm)
 * @param snr Signal-to-noise ratio(dBm)
 * @return None
 */
typedef void (*joinCallback)(bool isOk, int16_t rssi, int8_t snr);

/**
 * @fn rxCB
 * @brief The callback function for the node to receive data from the gateway.
 * @details The user-defined data reception callback function can obtain the received data and some network parameters.
 * @param buffer Received data
 * @param size The size of the received data
 * @param port Communication port
 * @param rssi Received Signal Strength Indication(dBm)
 * @param snr Signal-to-noise ratio(dBm)
 * @param ackReceived Whether it is an ACK
 * @param uplinkCounter The uplink frame count of the last transmission
 * @param downlinkCounter The downlink frame count of the last transmission
 * @return None
 */
typedef void (*rxCB)(void *buffer, uint16_t size, uint8_t port, int16_t rssi, int8_t snr, bool ackReceived, uint16_t uplinkCounter, uint16_t downlinkCounter);

/**
 * @fn txCB
 * @brief The callback function after the node sends data.
 * @details The user-defined data transmission callback function can obtain actual transmission power and other network parameters.
 * @param isconfirm Whether it is an acknowledgment packet
 * @param datarate Transmission rate, DR0~DR5
 * @param TxEirp Equivalent Isotropically Radiated Power(dBm)
 * @param Channel Transmission channel
 * @return None
 */
typedef void (*txCB)(bool isconfirm, int8_t datarate, int8_t TxEirp, uint8_t Channel);

class LoRaWAN_Node
{

public:

    /**
     * @fn LoRaWAN_Node
     * @brief OTAA mode node constructor.
     * @param devEui Device unique identifier, composed of 16 hexadecimal numbers
     * @param appEui Network identifier during the OTAA join process, composed of 16 hexadecimal numbers
     * @param appKey Application key, composed of 32 hexadecimal numbers
     * @param classType The operating mode of the node device, which can be CLASS_A or CLASS_C, with CLASS_A being the default
     * @n CLASS_A Basic mode
     * @n CLASS_C Continuous reception mode
     * @return OTAA mode node object
     */
    LoRaWAN_Node(const uint8_t *devEui, const uint8_t *appEui, const uint8_t *appKey, DeviceClass_t classType = CLASS_A);

    /**
     * @fn LoRaWAN_Node
     * @brief ABP mode node constructor.
     * @param devAddr Node device address, composed of 8 hexadecimal numbers
     * @param nwkSKey LoRaWAN network layer encryption key, composed of 32 hexadecimal numbers
     * @param appSKey LoRaWAN application layer encryption key, composed of 32 hexadecimal numbers
     * @param classType The operating mode of the node device, which can be CLASS_A or CLASS_C, with CLASS_A being the default
     * @n CLASS_A Basic mode
     * @n CLASS_C Continuous reception mode
     * @return OTAA mode node object
     */
    LoRaWAN_Node(const uint32_t devAddr, const uint8_t *nwkSKey, const uint8_t *appSKey, DeviceClass_t classType = CLASS_A);

    /**
     * @fn init
     * @brief LoRaWAN node initialization with specified parameters.
     * @param dataRate Node communication data rate
     * @param txEirp Equivalent Isotropically Radiated Power(dBm)
     * @param adr Whether the node has the adaptive data rate feature enabled, default is disabled
     * @param dutyCycle Whether duty cycle transmission limitation is enabled, which can be LORAWAN_DUTYCYCLE_ON or LORAWAN_DUTYCYCLE_OFF, 
     *                  with LORAWAN_DUTYCYCLE_OFF being the default.
     * @n LORAWAN_DUTYCYCLE_ON Enable duty cycle transmission limitation
     * @n LORAWAN_DUTYCYCLE_OFF Disable duty cycle transmission limitation
     * @return Whether the node initialization was successful
     * @retval true Initialization successful
     * @retval false Initialization failed
     */
    bool init(int8_t dataRate, int8_t txEirp, bool adr = false, bool dutyCycle = LORAWAN_DUTYCYCLE_OFF);

    /**
     * @fn join
     * @brief LoRaWAN node performs the network join operation and sets a user-defined join callback function.
     * @param callback User-defined join callback function, of type joinCallback
     * @return Whether the node actually performed the join operation
     * @retval 1 Actually performed the join operation
     * @retval 0 Actually did not perform the join operation
     */
    int join(joinCallback callback);

    /**
     * @fn isJoined
     * @brief Determine whether the node has joined the LoRaWAN network.
     * @param None
     * @return Whether the node has joined the LoRaWAN network
     * @retval true Already joined the network
     * @retval false Not yet joined the network
     */
    bool isJoined();

    /**
     * @fn getNetID
     * @brief Get the node's current network identifier, used to distinguish between different networks within the same region.
     * @param None
     * @return Current network identifier, composed of 6 hexadecimal numbers
     */
    uint32_t getNetID();

    /**
     * @fn getDevAddr
     * @brief Get the node device address.
     * @param None
     * @return Node device address, composed of 8 hexadecimal numbers
     */
    uint32_t getDevAddr();

    /**
     * @fn getDataRate
     * @brief Get the node's current communication data rate.
     * @param None
     * @return Node's current communication data rate
     */
    uint8_t getDataRate();

    /**
     * @fn getEIRP
     * @brief Get the node's equivalent isotropically radiated power(dBm)
     * @param None
     * @return Equivalent isotropically radiated power(dBm)
     */
    uint8_t getEIRP();

    /**
     * @fn getNwkSKey
     * @brief Get the node's LoRaWAN network layer encryption key.
     * @param None
     * @return Network layer encryption key, composed of 32 hexadecimal numbers
     */
    uint8_t* getNwkSKey();

    /**
     * @fn getAppSKey
     * @brief Get the node's LoRaWAN application layer encryption key.
     * @param None
     * @return Application layer encryption key, composed of 32 hexadecimal numbers
     */
    uint8_t* getAppSKey();

    /**
     * @fn getLastUplinkCounter
     * @brief Get the node's latest (previous) uplink frame counter.
     * @param None
     * @return Uplink frame counter
     */
    uint32_t getLastUplinkCounter();

    /**
     * @fn getLastDownCounter
     * @brief Get the gateway's latest (previous) downlink frame counter.
     * @param None
     * @return Downlink frame counter
     */
    uint32_t getLastDownCounter();

    /**
     * @fn deepSleepMs
     * @brief Set the MCU to immediately enter sleep for a specified duration.
     * @param timesleep Node sleep duration(ms).If set to 0, the device will never wake up.
     * @return None
     */
    void deepSleepMs(uint32_t timesleep);

    /**
     * @fn setRxCB
     * @brief Set the user-defined callback function for when the node receives data.
     * @param callback User-defined data reception callback function, of type rxHandler
     * @return Set result
     * @retval true Set successful
     * @retval false Set failed, please check if callback is NULL
     */
    bool setRxCB(rxCB callback);

    /**
     * @fn setTxCB
     * @brief Set the user-defined callback function for when the node sends data.
     * @param callback User-defined data transmission callback function, of type txHandler
     * @return Set result
     * @retval true Set successful
     * @retval false Set failed, please check if callback is NULL
     */
    bool setTxCB(txCB callback);
    
    /**
     * @fn sendConfirmedPacket
     * @brief Node sends data to the gateway in confirmed packet mode.
     * @param port Node communication port with the gateway
     * @param buffer Data to be sent by the node
     * @param size Size of the data to be sent by the node
     * @return Whether the node's request to send confirmed packet data was successful
     * @retval true Send successful
     * @retval false Send failed
     */
    bool sendConfirmedPacket(uint8_t port, void *buffer, uint8_t size);

    /**
     * @fn sendUnconfirmedPacket
     * @brief Node sends data to the gateway in unconfirmed packet mode.
     * @param port Node communication port with the gateway
     * @param buffer Data to be sent by the node
     * @param size Size of the data to be sent by the node
     * @return Whether the node's request to send unconfirmed packet data was successful
     * @retval true Send successful
     * @retval false Send failed
     */
    bool sendUnconfirmedPacket(uint8_t port, void *buffer, uint8_t size);

    /**
     * @fn setSubBand
     * @brief Set the frequency band for the US915 regional node.
     * @param subBand US915 region sub-band number (1–8, Default: 2).
     * @n The sub-band numbers correspond to channels. For specific frequency details of each channel, 
     *    refer to: ​​DFRobot_LoRaWAN\CHANNELS.MD
     * @n SubBand 1: channel 0-7, 64
     * @n SubBand 2: channel 8-15, 65
     * @n SubBand 3: channel 16-23, 66
     * @n SubBand 4: channel 24-31, 67
     * @n SubBand 5: channel 32-39, 68
     * @n SubBand 6: channel 40-47, 69
     * @n SubBand 7: channel 48-55, 70
     * @n SubBand 8: channel 56-63, 71
     * @return Whether the sub-band configuration was successful
     * @retval true Set successful
     * @retval false Set failed
     */
    bool setSubBand(uint8_t subBand);
    
    /**
     * @fn addChannel
     * @brief This method is not available.
     */
    bool addChannel(uint32_t freq);

    /**
     * @fn delChannel
     * @brief This method is not available.
     */
    bool delChannel(uint32_t freq);



private:

protected:

};
#endif