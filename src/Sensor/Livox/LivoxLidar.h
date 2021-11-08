/** Livox LiDAR data source, data from dependent lidar */

#ifndef LDS_LIDAR_H_
#define LDS_LIDAR_H_

#include "../../Base/BASE.h"
#include "../../Script/Kiss.h"

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
    kConfigImuRate = 8,
    kConfigScanPattern = 16,    
    kConfigLidarMode = 32,
} LidarConfigCodeBit;

typedef enum {
    kCoordinateCartesian = 0,
    kCoordinateSpherical
} CoordinateType;

typedef struct {
    bool enable_fan;
    LidarMode lidar_mode;
    uint32_t return_mode;
    uint32_t coordinate;
    uint32_t imu_rate;
    uint32_t scan_pattern;
    volatile uint32_t set_bits;
    volatile uint32_t get_bits;
} UserConfig;


typedef void ( *LivoxDataCallback ) ( LivoxEthPacket* pData, void* pLivox );

typedef struct {
    uint8_t handle;
    LidarConnectState connect_state;
    DeviceInfo info;
    UserConfig config;

    LivoxDataCallback pDataCb;
    void* pLivox;
} LidarDevice;

/**
 * LiDAR data source, data from dependent lidar.
 */
class LivoxLidar: public BASE
{
public:
    LivoxLidar();
    LivoxLidar ( const LivoxLidar& ) = delete;
    ~LivoxLidar();
    LivoxLidar& operator= ( const LivoxLidar& ) = delete;
    virtual bool init ( void* pKiss );

    static LivoxLidar& GetInstance()
    {
        static LivoxLidar lds_lidar;
        return lds_lidar;
    }

    int InitLdsLidar ( std::vector<std::string>& broadcast_code_strs );
    int DeInitLdsLidar ( void );

    bool setDataCallback ( const string& broadcastCode, LivoxDataCallback pCb, void* pLivox );
    bool setLidarMode ( const string& broadcastCode, LidarMode m);
    LidarDevice* findLidarDevice(const string& broadcastCode);

public:
    bool m_bEnableFan;
    uint32_t m_returnMode;
    uint32_t m_coordinate;
    uint32_t m_imuRate;
    uint32_t m_scanPattern;
    vector<string> m_vBroadcastCode;

private:
    int AddBroadcastCodeToWhitelist ( const char* broadcast_code );
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

    static void OnDeviceBroadcast   ( const BroadcastDeviceInfo *info );
    static void OnDeviceChange      ( const DeviceInfo *info, DeviceEvent type );
    static void DeviceInformationCb ( livox_status status, uint8_t handle, DeviceInformationResponse *ack, void *clent_data );
    static void LidarErrorStatusCb  ( livox_status status, uint8_t handle, ErrorMessage *message );

    static void GetLidarDataCb      ( uint8_t handle, LivoxEthPacket *data, uint32_t data_num, void *client_data );
    static void StartSampleCb       ( livox_status status, uint8_t handle, uint8_t response, void *clent_data );
    static void StopSampleCb        ( livox_status status, uint8_t handle, uint8_t response, void *clent_data );

    static void SetLidarModeCb   ( livox_status status, uint8_t handle, uint8_t response, void *clent_data );
    static void SetPointCloudReturnModeCb   ( livox_status status, uint8_t handle, uint8_t response, void *clent_data );
    static void SetCoordinateCb     ( livox_status status, uint8_t handle, uint8_t response, void *clent_data );
    static void SetImuRatePushFrequencyCb   ( livox_status status, uint8_t handle, uint8_t response, void *clent_data );
    static void SetScanPatternCb    ( livox_status status, uint8_t handle, DeviceParameterResponse *response, void *clent_data );
    static void ControlFanCb        ( livox_status status, uint8_t handle, uint8_t response, void *clent_data );

    bool auto_connect_mode_;
    uint32_t whitelist_count_;
    volatile bool is_initialized_;
    char broadcast_code_whitelist_[kMaxLidarCount][kBroadcastCodeSize];

    uint32_t lidar_count_;
    LidarDevice lidars_[kMaxLidarCount];
    uint32_t data_recveive_count_[kMaxLidarCount];
};

}
#endif
