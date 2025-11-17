/*!
 *@file DFRobot_LoRaWAN.cpp
 *@brief Define the basic structure of class LoRaWAN_Node, the implementation of basic methods.
 *@copyright Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 *@licence The MIT License (MIT)
 *@author [Martin](Martin@dfrobot.com)
 *@version V0.0.1
 *@date 2025-1-17
 *@get from https://www.dfrobot.com
 *@url https://github.com/DFRobot/DFRobot_LoRaWAN_ESP32S3
 */
#include <Arduino.h>
#include "DFRobot_LoRaWAN.h"
#include "mac/region/RegionCommon.h"
#include "boards/sx126x-board.h"
#include "mac/LoRaMacTest.h"
#include <rom/rtc.h>
#include <driver/rtc_io.h>
#include "apps/LoRaMac/common/LmHandler/LmHandler.h"
#include "mac/secure-element.h"

// 信号量
SemaphoreHandle_t loraIntSem = NULL;
SemaphoreHandle_t loraStateSem = NULL;

// lora任务接口
TaskHandle_t loraTaskHandle;

// 定义回调函数变量
static joinCallback loraJoinCb = NULL;
static rxCB rxCb = NULL;
static txCB txCb = NULL;

// 频道掩码相关变量，存放到非易失RTC缓存
RTC_DATA_ATTR uint16_t ChannelsMask[6];
RTC_DATA_ATTR uint16_t ChannelsDefaultMask[6];
RTC_DATA_ATTR uint16_t ChannelsMaskRemaining[6];

// 地区条件编译，从Arduino IDE中地区选项卡里设置
#ifdef REGION_EU868
#include "mac/region/RegionEU868.h"
LoRaMacRegion_t loraWanRegion = LORAMAC_REGION_EU868;
#endif
#ifdef REGION_CN470
#include "mac/region/RegionCN470.h"
LoRaMacRegion_t loraWanRegion = LORAMAC_REGION_CN470;
#endif
#ifdef REGION_US915
#include "mac/region/RegionUS915.h"
LoRaMacRegion_t loraWanRegion = LORAMAC_REGION_US915;
#endif

static void OnMacProcessNotify( void );
static void OnNetworkParametersChange( CommissioningParams_t* params );
static void OnMacMcpsRequest( LoRaMacStatus_t status, McpsReq_t *mcpsReq, TimerTime_t nextTxIn );
static void OnMacMlmeRequest( LoRaMacStatus_t status, MlmeReq_t *mlmeReq, TimerTime_t nextTxIn );
static void OnJoinRequest( LmHandlerJoinParams_t* params );
static void OnTxData( LmHandlerTxParams_t* params );
static void OnRxData( LmHandlerAppData_t* appData, LmHandlerRxParams_t* params );
static void OnClassChange( DeviceClass_t deviceClass );

static void startDeepSleep( void );

static uint8_t AppDataBuffer[256];                  // 数据包Buffer

static LmHandlerCallbacks_t LmHandlerCallbacks = 
{
    .GetBatteryLevel = BoardGetBatteryLevel,
    .GetTemperature = NULL,
    .GetRandomSeed = BoardGetRandomSeed,
    .OnMacProcess = OnMacProcessNotify,
    .OnNvmDataChange = NULL,
    .OnNetworkParametersChange = OnNetworkParametersChange,
    .OnMacMcpsRequest = OnMacMcpsRequest,
    .OnMacMlmeRequest = OnMacMlmeRequest,
    .OnJoinRequest = OnJoinRequest,
    .OnTxData = OnTxData,
    .OnRxData = OnRxData,
    .OnClassChange = OnClassChange,
    .OnBeaconStatusChange = NULL,
    .OnSysTimeUpdate = NULL,
};

static uint8_t DevEui_Default[] = LORAWAN_DEVICE_EUI;
static uint8_t AppEui_Default[] = LORAWAN_APPLICATION_EUI;
static uint8_t AppKey_Default[] = LORAWAN_APPLICATION_KEY;
static uint8_t AppSKey_Default[] = LORAWAN_APPSKEY; 
static uint8_t NwkSKey_Default[] = LORAWAN_NWKSKEY;

static LmHandlerParams_t LmHandlerParams = 
{
    .Region = loraWanRegion,
    .AdrEnable = LORAWAN_ADR_ON,
    .TxDatarate = LORAWAN_DEFAULT_DATARATE,
    .PublicNetworkEnable = LORAWAN_PUBLIC_NETWORK,

#ifdef DFROBOT_DUTYCYCLE_ON
    .DutyCycleEnabled = LORAWAN_DUTYCYCLE_ON,
#else
    .DutyCycleEnabled = LORAWAN_DUTYCYCLE_OFF,
#endif

    .DataBufferMaxSize = LORAWAN_APP_DATA_BUFFER_MAX_SIZE,
    .DataBuffer = AppDataBuffer,
    .TxEirp = 16,
    .joinType = ACTIVATION_TYPE_NONE,
    .DevEui = DevEui_Default,
    .JoinEui = AppEui_Default,
    .AppKey = AppKey_Default,
    .DevAddr = LORAWAN_DEVICE_ADDRESS,
    .AppSKey = AppSKey_Default,
    .NwkSKey = NwkSKey_Default,
    .NbTrials = 1,
    .Class = CLASS_A
};

/* ************************全局/静态函数定义************************** */

// MAC层正在处理radio中断通知函数 可以在这里改变一些变量的状态
static void OnMacProcessNotify( void )
{

}

// 用于通知应用层网络参数发生变化 比如协议栈初始化完成 可以在这里打出设置的DEVUI和JOINEUI是多少等等
static void OnNetworkParametersChange( CommissioningParams_t* params )
{
    // DisplayNetworkParametersUpdate( params );
}

// MCPS请求回调函数，可以在这里打出MCPS请求是否成功、请求类型、是否为确认包等等信息
static void OnMacMcpsRequest( LoRaMacStatus_t status, McpsReq_t *mcpsReq, TimerTime_t nextTxIn )
{
    // DisplayMacMcpsRequestUpdate( status, mcpsReq, nextTxIn );
}

// MLME请求回调函数，可以在这里打出MLME请求是否成功，比如入网包请求是否成功
static void OnMacMlmeRequest( LoRaMacStatus_t status, MlmeReq_t *mlmeReq, TimerTime_t nextTxIn )
{

    if( mlmeReq->Type == MLME_JOIN )
    {
        if( status != LORAMAC_STATUS_OK )
        {
            printf("\n\n-----------OTAA Send JOIN Req FAIL!------------\n\n");
            if (loraJoinCb != NULL) 
            { 
                loraJoinCb(false, 0, 0);                               
            }
        }
    }

}

// 入网请求回调
static void OnJoinRequest( LmHandlerJoinParams_t* params )
{
    // 在发入网包后更改速率DR为设定值
    MibRequestConfirm_t mibReq;
    mibReq.Type = MIB_CHANNELS_DATARATE;
    mibReq.Param.ChannelsDatarate = LmHandlerParams.TxDatarate;
    LoRaMacMibSetRequestConfirm(&mibReq);

    int16_t rssi = LoRaMacMcLastDataRssi();
    int8_t snr = LoRaMacMcLastDataSnr();

    if( params->CommissioningParams->IsOtaaActivation == true )     // OTAA入网通知
    {
        if( params->Status == LORAMAC_HANDLER_SUCCESS )
        {
            printf("\n\n-----------OTAA SUCCESS!----------\n\n");
            if (loraJoinCb != NULL) { loraJoinCb(true, rssi, snr); }

        }
        else                    
        {
            printf("\n\n-----------OTAA JOIN FAIL!------------\n\n");
            if (loraJoinCb != NULL) 
            { 
                
                loraJoinCb(false, rssi, snr);    
            }

        }

    }
    else   // ABP入网通知
    {
        printf("\n\n-----------ABP SUCCESS!------------\n\n");
        if (loraJoinCb != NULL) {
            loraJoinCb(true, rssi, snr); 
        }

    }

}

// 发送数据回调 可以在此处打出发送数据包的TxPower、频道等信息
static void OnTxData( LmHandlerTxParams_t* params )
{
    if(txCb != NULL && params != NULL)
    {
        uint8_t txeirp = 0;
#ifdef REGION_EU868
        txeirp = txpowerEirpEU868[params->TxPower][1];
#endif
#ifdef REGION_US915
        txeirp = txpowerEirpUS915[((params->TxPower < 4)?4:params->TxPower) - 4][1];
#endif
        txCb(params->AckReceived, params->Datarate, txeirp, params->Channel);
    }
}

// 接收数据回调
static void OnRxData( LmHandlerAppData_t* appData, LmHandlerRxParams_t* params )
{
    if (rxCb != NULL)
    {
        uint16_t uplinkcount = GetUplinkCounter();
        // uint16_t downlinkcount = GetDownlinkCounter();

        if(appData != NULL)
        {
            rxCb(
                appData->Buffer,
                appData->BufferSize,
                appData->Port,
                params->Rssi,
                params->Snr,
                params->IsRevACK,
                uplinkcount,
                params->DownlinkCounter
            );
        }
    }
}

// 设备工作模式切换通知
static void OnClassChange( DeviceClass_t deviceClass )
{
    // DisplayClassUpdate( deviceClass );
}


void loraTask(void *pvParameters)
{
    // printf("LoRa Task started\n");
    while (1)
    {
        if (xSemaphoreTake(loraIntSem, portMAX_DELAY) == pdTRUE)
        {   
            // printf("\n--------LmHandlerProcess ---------\n");
            LmHandlerProcess();
        }
    }
}

bool taskLoad(void)
{
    // Create the LoRaWan event semaphore
    // 二值 信号量用于同步
    loraIntSem = xSemaphoreCreateBinary();
    xSemaphoreGive(loraIntSem);
    xSemaphoreTake(loraIntSem, 10);

    // xTaskCreateUniversal(loraTask, "LORA", 8192, NULL, 1, &loraTaskHandle, ARDUINO_RUNNING_CORE);
    if (!xTaskCreate(loraTask, "LORA", 8192, NULL, 2, &loraTaskHandle))
    {
        return false;
    }
    return true;
}

static void startDeepSleep( void )
{
    SX126xIOInit();    // 预防用户没有初始化
    Radio.Standby();   // 容错
    Radio.Sleep();
    SetMacState(0);
    pinMode(LORA_SS, OUTPUT);
    digitalWrite(LORA_SS, HIGH);
    rtc_gpio_hold_en(gpio_num_t(LORA_SS));
    esp_deep_sleep_start();
}

/* ************************成员函数定义************************** */

LoRaWAN_Node::LoRaWAN_Node(const uint8_t *devEui, const uint8_t *appEui, const uint8_t *appKey, DeviceClass_t classType)
{
    memcpy(LmHandlerParams.DevEui, devEui, 8);
    memcpy(LmHandlerParams.JoinEui, appEui, 8);
    memcpy(LmHandlerParams.AppKey, appKey, 16);
    LmHandlerParams.joinType = ACTIVATION_TYPE_OTAA;
    LmHandlerParams.Class = classType;
}

LoRaWAN_Node::LoRaWAN_Node(const uint32_t devAddr, const uint8_t *nwkSKey, const uint8_t *appSKey, DeviceClass_t classType)
{
    memcpy(LmHandlerParams.NwkSKey, nwkSKey, 16);
    memcpy(LmHandlerParams.AppSKey, appSKey, 16);
    LmHandlerParams.DevAddr = devAddr;
    LmHandlerParams.joinType = ACTIVATION_TYPE_ABP;
    LmHandlerParams.Class = classType;
}

bool LoRaWAN_Node::init(int8_t dataRate, int8_t txEirp, bool adr, bool dutyCycle)
{
    // sx1262 IO初始化
    SX126xIOInit();

    // 915地区速率选择限制
#ifdef REGION_US915
    if (dataRate == DR_5 || dataRate == DR_6 || dataRate == DR_7)
    {
        printf("DDR_5 DR_6 DR_7 is not used on the US915\n");
        return false;
    }
#endif

    // lora任务创建
    taskLoad();

    // printf("\n\n\n--------LoRaWAN_Node::init   isJoined= %d--------------\n\n", isJoined());

    // 新 - 协议栈初始化
    LmHandlerParams.TxDatarate = dataRate;
    LmHandlerParams.TxEirp = txEirp;
    LmHandlerParams.AdrEnable = adr;
    LmHandlerParams.DutyCycleEnabled = dutyCycle;

    if(LmHandlerInit(&LmHandlerCallbacks, &LmHandlerParams) != LORAMAC_HANDLER_SUCCESS)
    {
        printf("\n\n\n--------------LmHandlerInit Failed!---------------\n\n");
        return false;
    }
    else
    {
#ifdef REGION_US915
    setSubBand(2);
#endif
        if(LmHandlerParams.joinType == ACTIVATION_TYPE_ABP)         // ABP模式相关参数在这里配置，用户无需在ABP模式调用join
        {
            MibRequestConfirm_t mibReq;
            mibReq.Type = MIB_NETWORK_ACTIVATION;
            mibReq.Param.NetworkActivation = ACTIVATION_TYPE_ABP;
            LoRaMacMibSetRequestConfirm(&mibReq);
        }

        // printf("\n\n\n--------------LmHandlerInit SUCCESS!---------------\n\n");
        return true;
    }    
}

void LoRaWAN_Node::deepSleepMs(uint32_t timesleep)
{
    if(timesleep != 0){
        // esp32进入指定睡眠时间，为0则不用定时器唤醒
        esp_sleep_enable_timer_wakeup(timesleep * (uint64_t)1000);
    }
    printf("\n\n------[API deepSleepMs] ESP32 Enter DeepSleep!------\n\n");
    startDeepSleep();
}

bool LoRaWAN_Node::setRxCB(rxCB callback)
{
    if(callback != NULL)
    {
        rxCb = callback;
        return true;
    }
    return false;
}

bool LoRaWAN_Node::setTxCB(txCB callback)
{
    if(callback != NULL)
    {
        txCb = callback;
        return true;
    }
    return false;
}

bool LoRaWAN_Node::isJoined()
{
    MibRequestConfirm_t mibreq; // 升级1.0.3
    mibreq.Type = MIB_NETWORK_ACTIVATION;
    LoRaMacMibGetRequestConfirm(&mibreq);

    if (mibreq.Param.NetworkActivation == ACTIVATION_TYPE_NONE)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool LoRaWAN_Node::setSubBand(uint8_t subBand)
{
    if(subBand < 1 || subBand > 8){
        return false;
    }
    uint16_t subBandChannelMask[6] = {0}, maxMask = 0;
    MibRequestConfirm_t mibreq;
    mibreq.Type = MIB_NVM_CTXS;
    LoRaMacMibGetRequestConfirm(&mibreq);
    LoRaMacNvmData_t *nvm = mibreq.Param.Contexts;
    switch (nvm->MacGroup2.Region)
    {
    case LORAMAC_REGION_CN470:
        maxMask = 6;
        if (subBand > 12) {
            return false;
        }
        subBandChannelMask[(subBand - 1) / 2] = ((subBand - 1) % 2) ? 0xFF00 : 0x00FF;
        // have not test yet! if meets any problems, see original logic below.
        break;
    case LORAMAC_REGION_AU915: // same as US915
    case LORAMAC_REGION_US915:
        maxMask = 6;
        if (subBand > 8) {
            return false;
        }
        subBandChannelMask[(subBand - 1) / 2] = ((subBand - 1) % 2) ? 0xFF00 : 0x00FF;
        subBandChannelMask[4]                 = 1 << (subBand - 1);
        break;
    default:
        return false;
    }
    RegionCommonChanMaskCopy(nvm->RegionGroup2.ChannelsDefaultMask, subBandChannelMask, maxMask);
    RegionCommonChanMaskCopy(nvm->RegionGroup2.ChannelsMask, subBandChannelMask, maxMask);
    RegionCommonChanMaskCopy(nvm->RegionGroup1.ChannelsMaskRemaining, subBandChannelMask, maxMask);

    return true;

}

bool LoRaWAN_Node::sendConfirmedPacket(uint8_t port, void *buffer, uint8_t size)     // 发送确认包
{   
    McpsReq_t mcpsReq;
    LoRaMacTxInfo_t txInfo;
    MibRequestConfirm_t mibReq;
    mibReq.Type = MIB_CHANNELS_DATARATE;
    LoRaMacMibGetRequestConfirm(&mibReq);

    if (LoRaMacQueryTxPossible(size, &txInfo) != LORAMAC_STATUS_OK)
    {
        // Send empty frame in order to flush MAC commands  发送空帧以刷新MAC命令
        mcpsReq.Type = MCPS_UNCONFIRMED;
        mcpsReq.Req.Unconfirmed.fBuffer = NULL;
        mcpsReq.Req.Unconfirmed.fBufferSize = 0;

        mcpsReq.Req.Unconfirmed.Datarate = mibReq.Param.ChannelsDatarate;
    }
    else
    {

        mcpsReq.Type = MCPS_CONFIRMED;
        mcpsReq.Req.Confirmed.fPort = port;
        mcpsReq.Req.Confirmed.fBuffer = (uint8_t *)buffer;
        mcpsReq.Req.Confirmed.fBufferSize = size;
        mcpsReq.Req.Confirmed.NbTrials = LmHandlerParams.NbTrials;
        mcpsReq.Req.Confirmed.Datarate = mibReq.Param.ChannelsDatarate;
    }


    // if(LmHandlerParams.joinType == ACTIVATION_TYPE_ABP)
    // {
    //     MibRequestConfirm_t mibReq;
    //     // appskey
    //     mibReq.Type = MIB_APP_S_KEY;
    //     mibReq.Param.AppSKey = LmHandlerParams.AppSKey;
    //     LoRaMacMibSetRequestConfirm(&mibReq);

    //     // nwkskey
    //     mibReq.Type = MIB_S_NWK_S_INT_KEY;
    //     mibReq.Param.SNwkSIntKey = LmHandlerParams.NwkSKey;
    //     LoRaMacMibSetRequestConfirm(&mibReq);
    //     mibReq.Type = MIB_NWK_S_ENC_KEY;
    //     mibReq.Param.NwkSEncKey = LmHandlerParams.NwkSKey;
    //     LoRaMacMibSetRequestConfirm(&mibReq);
    // }
    
// printf("-----------LoRaWAN_Node::sendConfirmedPacket 1 step------------\n");
    LoRaMacStatus_t status = LoRaMacMcpsRequest(&mcpsReq);
    if (status == LORAMAC_STATUS_OK)
    {
        // printf("-----------LoRaMacMcpsRequest LORAMAC_STATUS_OK------------\n");
        return true;
    }
    // printf("-----------LoRaMacMcpsRequest ERROR code = %d------------\n", status);
// printf("-----------LoRaWAN_Node::sendConfirmedPacket 3 step------------\n");
    return false;
}

bool LoRaWAN_Node::sendUnconfirmedPacket(uint8_t port, void *buffer, uint8_t size)
{
    McpsReq_t mcpsReq;
    LoRaMacTxInfo_t txInfo;
    MibRequestConfirm_t mibReq;
    mibReq.Type = MIB_CHANNELS_DATARATE;
    LoRaMacMibGetRequestConfirm(&mibReq);
    if (LoRaMacQueryTxPossible(size, &txInfo) != LORAMAC_STATUS_OK)
    {
        // Send empty frame in order to flush MAC commands
        mcpsReq.Type = MCPS_UNCONFIRMED;
        mcpsReq.Req.Unconfirmed.fBuffer = NULL;
        mcpsReq.Req.Unconfirmed.fBufferSize = 0;
        mcpsReq.Req.Unconfirmed.Datarate = mibReq.Param.ChannelsDatarate;
    }
    else
    {
        mcpsReq.Type = MCPS_UNCONFIRMED;
        mcpsReq.Req.Unconfirmed.fPort = port;
        mcpsReq.Req.Unconfirmed.fBuffer = (uint8_t *)buffer;
        mcpsReq.Req.Unconfirmed.fBufferSize = size;
        mcpsReq.Req.Unconfirmed.Datarate = mibReq.Param.ChannelsDatarate;
    }

    if (LoRaMacMcpsRequest(&mcpsReq) == LORAMAC_STATUS_OK)
    {
        return false;
    }
    return true;
}

int LoRaWAN_Node::join(joinCallback callback)
{
    loraJoinCb = callback;

    if (isJoined()) {
        return 0;
    }   

    // 新入网逻辑
    LmHandlerJoin();

    return 1;
}

uint32_t LoRaWAN_Node::getDevAddr()
{
    // return LoRaMacGetOTAADevId();
    MibRequestConfirm_t mibReq;
    mibReq.Type = MIB_DEV_ADDR;
    LoRaMacMibGetRequestConfirm( &mibReq );
    return mibReq.Param.DevAddr;
}

uint8_t LoRaWAN_Node::getDataRate()
{
    MibRequestConfirm_t mibReq;
    mibReq.Type = MIB_CHANNELS_DATARATE;
    LoRaMacMibGetRequestConfirm(&mibReq);

    return mibReq.Param.ChannelsDatarate;
}

uint8_t LoRaWAN_Node::getEIRP()
{
    MibRequestConfirm_t mibReq;
    mibReq.Type = MIB_CHANNELS_TX_POWER;
    LoRaMacMibGetRequestConfirm(&mibReq);
    uint8_t txeirp = 0;

#ifdef REGION_EU868
    txeirp = txpowerEirpEU868[mibReq.Param.ChannelsTxPower][1];
#endif

#ifdef REGION_US915
    txeirp = txpowerEirpUS915[((mibReq.Param.ChannelsTxPower < 4)?4:mibReq.Param.ChannelsTxPower) - 4][1];
#endif

    return txeirp;
}

bool LoRaWAN_Node::addChannel(uint32_t freq)
{

    uint8_t id = 0;
    uint8_t chanIdx = 0;
    uint8_t i = 0;
    uint8_t j = 0;
    for (uint8_t k = 0; k < 96; k++) {
        if (((ChannelsMask[i] >> j) & 0x01) == 0x01) {
            id++;
        } else {
            chanIdx = id;
            break;
        }
        if (j == 15) {
            j = 0;
            i++;
        } else {
            j++;
        }
    }
    printf("ChannelsMask1 0x%x\n", ChannelsMask[0]);
    printf("ChannelsMask2 0x%x\n", ChannelsMask[1]);
    ChannelAddParams_t channelAdd;
    ChannelParams_t newChannel;

    newChannel.Frequency = freq;

    // printf("Apply CF list: new channel at Freq = %d\n", newChannel.Frequency);
    //  Initialize alternative frequency to 0
    newChannel.Rx1Frequency = 0;
    newChannel.DrRange = {((DR_5 << 4) | DR_0)};
    newChannel.Band = 1;
    channelAdd.NewChannel = &newChannel;
    channelAdd.ChannelId = chanIdx;
    printf("id = %d\n", chanIdx);
#ifdef REGION_EU868
    if (RegionEU868ChannelAdd(&channelAdd) == LORAMAC_STATUS_OK) {
        return true;
    }
#endif
    return false;
}

bool LoRaWAN_Node::delChannel(uint32_t freq)
{
    ChannelRemoveParams_t channelRemove;
#ifdef REGION_EU868
    channelRemove.ChannelId = getEU868FrqID(freq);
    if (RegionEU868ChannelsRemove(&channelRemove) == LORAMAC_STATUS_OK) {
        return true;
    }
#endif
    return false;
}

uint32_t LoRaWAN_Node::getNetID()
{
    MibRequestConfirm_t mibReq;
    mibReq.Type = MIB_NET_ID;
    LoRaMacMibGetRequestConfirm(&mibReq);
    return mibReq.Param.NetID;
}

uint8_t *LoRaWAN_Node::getAppSKey()
{
    uint8_t* appskey = SecureElementGetAppskey();
    if(appskey == nullptr)
    {
        printf("\nGet Appskey Fail!\n");
        return nullptr;
    }
    return appskey;    
}

uint8_t *LoRaWAN_Node::getNwkSKey()
{
    uint8_t* nwkskey = SecureElementGetNwkskey();
    if(nwkskey == nullptr)
    {
        printf("\nGet Nwkskey Fail!\n");
        return nullptr;
    }
    return nwkskey;
}

uint32_t LoRaWAN_Node::getLastUplinkCounter()
{
    return GetUplinkCounter();
}

uint32_t LoRaWAN_Node::getLastDownCounter()
{
    return GetDownlinkCounter();
}
