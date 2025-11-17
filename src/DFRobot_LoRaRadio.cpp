/*!
 *@file DFRobot_LoRaRadio.cpp
 *@brief Define the basic structure of class DFRobot_LoRaRadio, the implementation of basic methods.
 *@copyright Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 *@licence The MIT License (MIT)
 *@author [Martin](Martin@dfrobot.com)
 *@version V0.0.1
 *@date 2025-3-14
 *@get from https://www.dfrobot.com
 *@url https://github.com/DFRobot/DFRobot_LoRaWAN_ESP32S3
*/
#include "DFRobot_LoRaRadio.h"
#include "radio/radio.h"
#include "radio/sx126x/sx126x.h"
#include "boards/sx126x-board.h"
#include "mac/LoRaMacCrypto.h"
#include <Arduino.h>
#include <driver/rtc_io.h>

static RadioEvents_t radioEvent;            // radio层驱动回调
extern SemaphoreHandle_t loraIntSem;
extern TaskHandle_t loraTaskHandle;

static rxCB *rxEncryptiondone = NULL;

uint8_t  isEncryption = false;      // 是否加密传输
uint8_t  dataKey[16];               // 数据密钥

void loraRxCb(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
    if(rxEncryptiondone == NULL)
    {
        printf("rxEncryptiondone NULL");
        return;
    }
    
    if(isEncryption == false)
    {
        rxEncryptiondone(payload,size,rssi,snr);
    } 
    else
    {
        uint8_t* dataread = (uint8_t*)malloc(sizeof(uint8_t)*size+1);
        LoRaMacPayloadDecrypt(payload,
                            size,
                            dataKey,
                            0xDFDFDFDF,
                            1,
                            0X66,
                            dataread);
        rxEncryptiondone(dataread,size,rssi,snr);
        free(dataread);
    }
}

DFRobot_LoRaRadio::DFRobot_LoRaRadio(){
}

void loraRadioTask(void *pvParameters)
{
    //printf("LoRa Task started\n");
    while (1)
    {
        if (xSemaphoreTake(loraIntSem, portMAX_DELAY) == pdTRUE)
        {
            // Handle Radio2 events
            Radio2.BgIrqProcess();
        }
    }
}

bool lorataskLoad(void)
{
    // Create the LoRaWan event semaphore
    //二值 信号量用于同步
    loraIntSem = xSemaphoreCreateBinary();

    xSemaphoreGive(loraIntSem);
    xSemaphoreTake(loraIntSem, 10);
    //xTaskCreateUniversal(loraRadioTask, "LORA", 8192, NULL, 1, &loraTaskHandle, ARDUINO_RUNNING_CORE);
    if (!xTaskCreate(loraRadioTask, "LORA", 8192, NULL, 2, &loraTaskHandle))
    {
        return false;
    }
    return true;
}

void DFRobot_LoRaRadio::init()
{
    SX126xIOInit();
    uint16_t readSyncWord = 0;
    SX126xReadRegisters(REG_LR_SYNCWORD, (uint8_t*)&readSyncWord, 2);
    printf("SyncWord = %04X\n", readSyncWord);
    
    lorataskLoad();  
    Radio2.Init(&radioEvent);
}

void DFRobot_LoRaRadio::setBW(eBandwidths_t BW)
{
    _bandwidth = BW;
    Radio2.SetTxConfig(MODEM_LORA, _txeirp, 0,(uint32_t)_bandwidth, _SF, 1, 8, false, true, 0, 0, false, 3000);
    Radio2.SetRxConfig(MODEM_LORA,(uint32_t)_bandwidth, _SF, 1, 0, 8, 0, false, 0, true, 0, 0, false, true);
    delay(10);
}

void DFRobot_LoRaRadio::setEIRP(int8_t EIRP)
{
    _txeirp = EIRP;
    Radio2.SetTxConfig(MODEM_LORA, _txeirp, 0,(uint32_t)_bandwidth, _SF, 1, 8, false, true, 0, 0, false, 3000);
    delay(10);
}

void DFRobot_LoRaRadio::setSF(uint8_t SF)
{
    _SF = SF;
    Radio2.SetTxConfig(MODEM_LORA, _txeirp, 0,(uint32_t)_bandwidth, _SF, 1, 8, false, true, 0, 0, false, 3000);
    Radio2.SetRxConfig(MODEM_LORA,(uint32_t)_bandwidth, _SF, 1, 0, 8, 0, false, 0, true, 0, 0, false, true);
    delay(10);
}

void DFRobot_LoRaRadio::setSync(uint16_t sync)
{
    // uint8_t buffer[2];
    // buffer[0] = sync>>8;
    // buffer[1] = sync & 0xFF;
    // printf("set SyncWord = %04X\n", sync);
    // SX126xWriteRegisters(REG_LR_SYNCWORD, buffer, 2);

    // uint16_t readSyncWord = 0;
    // SX126xReadRegisters(REG_LR_SYNCWORD, (uint8_t*)&readSyncWord, 2);
    // printf("read SyncWord = %04X\n", readSyncWord);
}

void DFRobot_LoRaRadio::setFreq(uint32_t freq)
{
    //SX126xSetRfFrequency(freq);
    Radio2.SetChannel(freq);
}

void DFRobot_LoRaRadio::sendData(const void *data, uint8_t size)
{
    if(isEncryption == true)
    {
       uint8_t* dataSend = (uint8_t*)malloc(sizeof(uint8_t)*size + 1);
       LoRaMacPayloadEncrypt((uint8_t *)data, size, dataKey, 0xDFDFDFDF, 1, 0X66, dataSend);
       Radio2.Send((uint8_t*)dataSend, size);
       free(dataSend);
    } 
    else 
    {
       Radio2.Send((uint8_t*)data, size);
    }
}

void DFRobot_LoRaRadio::setTxCB(txCB cb)
{
   // txdone = cb;
    radioEvent.TxDone  = cb;
    reInitEvent(&radioEvent);
}

void DFRobot_LoRaRadio::setRxCB(rxCB cb)
{
    rxEncryptiondone = cb;
    radioEvent.RxDone  = loraRxCb;
    reInitEvent(&radioEvent);
}

void DFRobot_LoRaRadio::startRx()
{ 
    Radio2.Rx(0xFFFFFF);
}

void DFRobot_LoRaRadio::stopRx()
{
    Radio2.Standby();
}

void DFRobot_LoRaRadio::setCadCB(cadDoneCB cb)
{   
    radioEvent.CadDone  = cb;
    reInitEvent(&radioEvent);
}

void DFRobot_LoRaRadio::setRxErrorCB(rxErrorCB cb)
{
    radioEvent.RxError  = cb;
    reInitEvent(&radioEvent);
}

void DFRobot_LoRaRadio::startCad(RadioLoRaCadSymbols_t cadSymbolNum, uint8_t cadDetPeak, uint8_t cadDetMin)
{
    // printf("startCad\n");
    //Radio2.Standby();
    Radio2.SetCadParams(cadSymbolNum, cadDetPeak, cadDetMin, LORA_CAD_ONLY, 300000);
    //cadTime = millis();
    Radio2.StartCad();
}

void DFRobot_LoRaRadio::deepSleepMs(uint32_t timesleep)
{
    if(timesleep != 0){
        // esp32进入指定睡眠时间，为0则不用定时器唤醒
        esp_sleep_enable_timer_wakeup(timesleep * (uint64_t)1000);
    }
    printf("\n\n------[API deepSleepMs] ESP32 Enter DeepSleep!------\n\n");
    SX126xIOInit();     // 预防用户没有初始化
    Radio2.Standby();   // 容错
    Radio2.Sleep();
    pinMode(LORA_SS, OUTPUT);
    digitalWrite(LORA_SS, HIGH);
    rtc_gpio_hold_en(gpio_num_t(LORA_SS));
    esp_deep_sleep_start();
}

void DFRobot_LoRaRadio::setEncryptKey(const uint8_t *key)
{
    isEncryption = true;
    memcpy(dataKey,key,16);
}

void DFRobot_LoRaRadio::dumpRegisters()
{
    static const uint16_t regs[] = {
        0x0580,0x0583,  0x0584,0x0585,  0x06B8,0x06B9,  0x06BC,0x06BD,
        0x06BE,0x06BF,  0x06C0,0x06C1,  0x06C2,0x06C3,  0x06C4,0x06C5,
        0x06C6,0x06C7,  0x06CD,0x06CE,  0x0736,0x0740,  0x0741,0x0819,
        0x081A,0x081B,  0x081C,0x0889,  0x08AC,0x08D8,  0x08E7,0x0902,
        0x0911,0x0912,  0x0920,0x0944};

    printf("------register list ----------------\n");
    for(int i = 0; i < sizeof(regs)/sizeof(regs[0]); i++)
    {
        printf("Reg 0x%04X = 0X%02X\n", regs[i], SX126xReadRegister(regs[i]));
    }
    printf("------------------------------------\n");
}