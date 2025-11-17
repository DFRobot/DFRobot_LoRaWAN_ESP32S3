# DFRobot_LoRaWAN_ESP32S3

DFRobot_LoRaWAN_ESP32S3 is an Arduino communication library developed for the DFR1195 node, enabling the node to communicate with the gateway using the LoRaWAN 1.0.3 protocol stack. It also integrates the DFRobot_LoRaRadio library, allowing for LoRa point-to-point communication between nodes.

## Product Link 

    URL: https://www.dfrobot.com
    SKU: DFR1195

## Table of Contents

* [Summary](#summary)
* [Installation](#installation)
* [DFRobot_LoRaWAN Methods](#DFRobot_LoRaWAN methods)
* [DFRobot_LoRaRadio Methods](#DFRobot_LoRaRadio Methods)
* [Compatibility](#compatibility)
* [History](#history)
* [Credits](#credits)

## Summary

Using this library allows end nodes to join a LoRaWAN network via OTAA or ABP, communicate with the gateway using confirmed or unconfirmed packets, and enables multi-node communication in LoRaRadio mode. Both LoRaWAN and LoRaRadio communication modes support low power consumption (µA). 

## Installation

To use this library, first download the library file, paste it into the \Arduino\libraries directory, then open the examples folder and run the demo in the folder. 

## DFRobot_LoRaWAN Methods

```C++
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
     * @param dutyCycle Whether duty cycle transmission limitation is enabled, which can be LORAWAN_DUTYCYCLE_ON or LORAWAN_DUTYCYCLE_OFF, with LORAWAN_DUTYCYCLE_OFF being the default.
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
     * @n The sub-band numbers correspond to channels. For specific frequency details of each channel, refer to: DFRobot_LoRaWAN\CHANNELS.MD
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
```

## DFRobot_LoRaRadio Methods

```C++
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
     * @fn setRxTimeOutCB
     * @brief Sets the callback function for when data reception times out.
     * @param cb The callback
     * @return None
     */
    void setRxTimeOutCB(rxTimeOutCB cb);

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
```

## Compatibility

| MCU         | Work Well | Work Wrong | Untested | Remarks |
| ----------- | :-------: | :--------: | :------: | ------- |
| Arduino uno |          |            |          |         |
| Mega2560    |          |            |          |         |
| Leonardo    |          |            |          |         |
| ESP32       |    √    |            |          |         |
| micro:bit   |          |            |          |         |

## History

- 2025/11/17 - Version 1.0.0 released.

## Credits

Written by Martin(Martin@dfrobot.com), 2025. (Welcome to our [website](https://www.dfrobot.com/))
