/*
 * _Livox.h
 *
 *  Created on: Aug 21, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Sensor__Livox_H_
#define OpenKAI_src_Sensor__Livox_H_

#include "../Base/_ModuleBase.h"

#ifdef USE_LIVOX
#include "livox_def.h"
#include "livox_sdk.h"

namespace kai
{

typedef enum {
    kConnectStateOff = 0,
    kConnectStateOn = 1,
    kConnectStateConfig = 2,
    kConnectStateSampling = 3,
} LidarConnectState;

typedef enum {
    kConfigFan = 1,
    kConfigReturnMode = 2,
    kConfigCoordinate = 4,
    kConfigImuRate = 8
} LidarConfigCodeBit;

typedef enum {
    kCoordinateCartesian = 0,
    kCoordinateSpherical
} CoordinateType;

typedef struct {
    bool enable_fan;
    uint32_t return_mode;
    uint32_t coordinate;
    uint32_t imu_rate;
    volatile uint32_t set_bits;
    volatile uint32_t get_bits;
} UserConfig;

typedef struct {
    uint8_t handle;
    LidarConnectState connect_state;
    DeviceInfo info;
    UserConfig config;
} LidarDevice;


class _Livox: public _ModuleBase
{
public:
    _Livox();
    ~_Livox();

    bool init ( void* pKiss );
    void close ( void );
    bool start ( void );
    void draw ( void );

private:
    bool open ( void );
    bool updateLidar ( void );
    void update ( void );
    static void* getUpdate ( void* This )
    {
        ( ( _Livox* ) This )->update();
        return NULL;
    }

    static void GetLidarDataCb ( uint8_t handle, LivoxEthPacket *data, uint32_t data_num, void *client_data );
    static void OnDeviceBroadcast ( const BroadcastDeviceInfo *info );
    static void OnDeviceChange ( const DeviceInfo *info, DeviceEvent type );
    static void StartSampleCb ( livox_status status, uint8_t handle, uint8_t response, void *clent_data );
    static void StopSampleCb ( livox_status status, uint8_t handle, uint8_t response, void *clent_data );
    static void DeviceInformationCb ( livox_status status, uint8_t handle, DeviceInformationResponse *ack, void *clent_data );
    static void LidarErrorStatusCb ( livox_status status, uint8_t handle, ErrorMessage *message );
    static void ControlFanCb ( livox_status status, uint8_t handle, uint8_t response, void *clent_data );
    static void SetPointCloudReturnModeCb ( livox_status status, uint8_t handle, uint8_t response, void *clent_data );
    static void SetCoordinateCb ( livox_status status, uint8_t handle, uint8_t response, void *clent_data );
    static void SetImuRatePushFrequencyCb ( livox_status status, uint8_t handle, uint8_t response, void *clent_data );

    int AddBroadcastCodeToWhitelist ( const char* broadcast_code );
    void AddLocalBroadcastCode ( void );
    bool FindInWhitelist ( const char* broadcast_code );

    void EnableAutoConnectMode ( void )
    {
        auto_connect_mode_ = true;
    }
    void DisableAutoConnectMode ( void )
    {
        auto_connect_mode_ = false;
    }
    bool IsAutoConnectMode ( void )
    {
        return auto_connect_mode_;
    }

    bool auto_connect_mode_;
    uint32_t whitelist_count_;
    volatile bool is_initialized_;
    char broadcast_code_whitelist_[kMaxLidarCount][kBroadcastCodeSize];

    uint32_t lidar_count_;
    LidarDevice lidars_[kMaxLidarCount];
    uint32_t data_recveive_count_[kMaxLidarCount];

public:
    bool m_bOpen;
    vector<string> m_vBroadcastCode;

};

}
#endif
#endif
