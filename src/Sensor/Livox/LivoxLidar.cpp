#ifdef USE_LIVOX

#include "LivoxLidar.h"

#include <stdio.h>
#include <string.h>
#include <thread>
#include <memory>

namespace kai
{

    /** Const varible ------------------------------------------------------------------------------- */
    /** User add broadcast code here */
    static const char *local_broadcast_code_list[] =
    {
            "000000000000001",
    };

    /** For callback use only */
    LivoxLidar *g_pLidar = nullptr;

    /** Lds lidar function ---------------------------------------------------------------------------*/
    LivoxLidar::LivoxLidar()
    {
        auto_connect_mode_ = true;
        whitelist_count_ = 0;
        is_initialized_ = false;

        lidar_count_ = 0;
        memset(broadcast_code_whitelist_, 0, sizeof(broadcast_code_whitelist_));

        memset(lidars_, 0, sizeof(lidars_));
        for (uint32_t i = 0; i < kMaxLidarCount; i++)
        {
            lidars_[i].handle = kMaxLidarCount;
            /** Unallocated state */
            lidars_[i].connect_state = kConnectStateOff;

            lidars_[i].pDataCb = NULL;
            lidars_[i].pLivox = NULL;
        }

        m_bEnableFan = true;
        m_returnMode = kStrongestReturn;
        m_coordinate = kCoordinateCartesian;
        m_imuRate = kImuFreq200Hz;
        m_scanPattern = kNoneRepetitiveScanPattern;
    }

    LivoxLidar::~LivoxLidar()
    {
    }

    bool LivoxLidar::init(void *pKiss)
    {
        IF_F(!this->BASE::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->a("vBroadcastCode", &m_vBroadcastCode);
        pK->v("bEnableFan", &m_bEnableFan);
        pK->v("returnMode", &m_returnMode);
        pK->v("coordinate", &m_coordinate);
        pK->v("imuRate", &m_imuRate);
        pK->v("scanPattern", &m_scanPattern);

        int r = InitLdsLidar(m_vBroadcastCode);

        IF_F(r != 0);
        return true;
    }

    int LivoxLidar::InitLdsLidar(std::vector<std::string> &broadcast_code_strs)
    {
        if (is_initialized_)
        {
            printf("LiDAR data source is already inited!\n");
            return 0; //-1;
        }

        if (!Init())
        {
            Uninit();
            printf("Livox-SDK init fail!\n");
            return -1;
        }

        LivoxSdkVersion _sdkversion;
        GetLivoxSdkVersion(&_sdkversion);
        printf("Livox SDK version %d.%d.%d\n", _sdkversion.major, _sdkversion.minor, _sdkversion.patch);

        SetBroadcastCallback(LivoxLidar::OnDeviceBroadcast);
        SetDeviceStateUpdateCallback(LivoxLidar::OnDeviceChange);

        /** Add commandline input broadcast code */
        for (auto input_str : broadcast_code_strs)
        {
            LivoxLidar::AddBroadcastCodeToWhitelist(input_str.c_str());
        }

        /** Add local broadcast code */
        LivoxLidar::AddLocalBroadcastCode();

        if (whitelist_count_)
        {
            LivoxLidar::DisableAutoConnectMode();
            printf("Disable auto connect mode!\n");

            printf("List all broadcast code in whiltelist:\n");
            for (uint32_t i = 0; i < whitelist_count_; i++)
            {
                printf("%s\n", broadcast_code_whitelist_[i]);
            }
        }
        else
        {
            LivoxLidar::EnableAutoConnectMode();
            printf("No broadcast code was added to whitelist, swith to automatic connection mode!\n");
        }

        /** Add here, only for callback use */
        g_pLidar = this;

        /** Start livox sdk to receive lidar data */
        if (!Start())
        {
            Uninit();
            printf("Livox-SDK init fail!\n");
            return -1;
        }

        is_initialized_ = true;
        printf("Livox-SDK init success!\n");

        return 0;
    }

    int LivoxLidar::DeInitLdsLidar(void)
    {

        if (!is_initialized_)
        {
            printf("LiDAR data source is not exit");
            return -1;
        }

        Uninit();
        printf("Livox SDK Deinit completely!\n");

        return 0;
    }

    bool LivoxLidar::setDataCallback(const string &broadcastCode, LivoxDataCallback pCb, void *pLivox)
    {
        NULL_F(pLivox);

        LidarDevice* pL = findLidarDevice(broadcastCode);
        NULL_F(pL);

        pL->pDataCb = pCb;
        pL->pLivox = pLivox;
        return true;
    }

    bool LivoxLidar::setLidarMode(const string &broadcastCode, LidarMode m)
    {
        LidarDevice* pL = findLidarDevice(broadcastCode);
        NULL_F(pL);

        pL->config.lidar_mode = m;
        LidarSetMode(pL->handle, pL->config.lidar_mode, LivoxLidar::SetLidarModeCb, g_pLidar);
    }

    LidarDevice* LivoxLidar::findLidarDevice(const string& broadcastCode)
    {
        for (int i = 0; i < kMaxLidarCount; i++)
        {
            LidarDevice *pL = &lidars_[i];
            string bCode = string(pL->info.broadcast_code);
            IF_CONT(broadcastCode != bCode);

            return pL;
        }

        return NULL;
    }

    /** Static function in LdsLidar for callback or event process ------------------------------------*/

    /** Receiving point cloud data from Livox LiDAR. */
    void LivoxLidar::GetLidarDataCb(uint8_t handle, LivoxEthPacket *data,
                                    uint32_t data_num, void *client_data)
    {
        using namespace std;

        LivoxLidar *lidar_this = static_cast<LivoxLidar *>(client_data);
        LivoxEthPacket *eth_packet = data;

        if (!data || !data_num || (handle >= kMaxLidarCount))
        {
            return;
        }

        LidarDevice *pLdev = &lidar_this->lidars_[handle];
        if (!pLdev->pDataCb)
            return;
        pLdev->pDataCb(data, pLdev->pLivox);
    }

    void LivoxLidar::OnDeviceBroadcast(const BroadcastDeviceInfo *info)
    {
        if (info == nullptr)
        {
            return;
        }

        if (info->dev_type == kDeviceTypeHub)
        {
            printf("In lidar mode, couldn't connect a hub : %s\n", info->broadcast_code);
            return;
        }

        if (g_pLidar->IsAutoConnectMode())
        {
            printf("In automatic connection mode, will connect %s\n", info->broadcast_code);
        }
        else
        {
            if (!g_pLidar->FindInWhitelist(info->broadcast_code))
            {
                printf("Not in the whitelist, please add %s to if want to connect!\n",
                       info->broadcast_code);
                return;
            }
        }

        livox_status result = kStatusFailure;
        uint8_t handle = 0;
        result = AddLidarToConnect(info->broadcast_code, &handle);
        if (result == kStatusSuccess && handle < kMaxLidarCount)
        {
            SetDataCallback(handle, LivoxLidar::GetLidarDataCb, (void *)g_pLidar);
            LidarDevice *p_lidar = &(g_pLidar->lidars_[handle]);
            p_lidar->handle = handle;
            p_lidar->connect_state = kConnectStateOff;
            p_lidar->config.enable_fan = g_pLidar->m_bEnableFan;
            p_lidar->config.return_mode = g_pLidar->m_returnMode;
            p_lidar->config.coordinate = g_pLidar->m_coordinate;
            p_lidar->config.imu_rate = g_pLidar->m_imuRate;
            p_lidar->config.scan_pattern = g_pLidar->m_scanPattern;
        }
        else
        {
            printf("Add lidar to connect is failed : %d %d \n", result, handle);
        }
    }

    /** Callback function of changing of device state. */
    void LivoxLidar::OnDeviceChange(const DeviceInfo *info, DeviceEvent type)
    {
        if (info == nullptr)
        {
            return;
        }

        uint8_t handle = info->handle;
        if (handle >= kMaxLidarCount)
        {
            return;
        }

        LidarDevice *p_lidar = &(g_pLidar->lidars_[handle]);
        if (type == kEventConnect)
        {
            QueryDeviceInformation(handle, DeviceInformationCb, g_pLidar);
            if (p_lidar->connect_state == kConnectStateOff)
            {
                p_lidar->connect_state = kConnectStateOn;
                p_lidar->info = *info;
            }
            printf("[WARNING] Lidar sn: [%s] Connect!!!\n", info->broadcast_code);
        }
        else if (type == kEventDisconnect)
        {
            p_lidar->connect_state = kConnectStateOff;
            printf("[WARNING] Lidar sn: [%s] Disconnect!!!\n", info->broadcast_code);
        }
        else if (type == kEventStateChange)
        {
            p_lidar->info = *info;
            printf("[WARNING] Lidar sn: [%s] StateChange!!!\n", info->broadcast_code);
        }

        if (p_lidar->connect_state == kConnectStateOn)
        {
            printf("Device Working State %d\n", p_lidar->info.state);
            if (p_lidar->info.state == kLidarStateInit)
            {
                printf("Device State Change Progress %u\n", p_lidar->info.status.progress);
            }
            else
            {
                printf("Device State Error Code 0X%08x\n", p_lidar->info.status.status_code.error_code);
            }
            printf("Device feature %d\n", p_lidar->info.feature);
            SetErrorMessageCallback(handle, LivoxLidar::LidarErrorStatusCb);

            /** Config lidar parameter */
            if (p_lidar->info.state == kLidarStateNormal)
            {
                if (p_lidar->config.coordinate != 0)
                {
                    SetSphericalCoordinate(handle, LivoxLidar::SetCoordinateCb, g_pLidar);
                }
                else
                {
                    SetCartesianCoordinate(handle, LivoxLidar::SetCoordinateCb, g_pLidar);
                }
                p_lidar->config.set_bits |= kConfigCoordinate;

                if (kDeviceTypeLidarMid40 != info->type)
                {
                    LidarSetPointCloudReturnMode(handle, (PointCloudReturnMode)(p_lidar->config.return_mode),
                                                 LivoxLidar::SetPointCloudReturnModeCb, g_pLidar);
                    p_lidar->config.set_bits |= kConfigReturnMode;
                }

                if (kDeviceTypeLidarMid40 != info->type && kDeviceTypeLidarMid70 != info->type)
                {
                    LidarSetImuPushFrequency(handle, (ImuFreq)(p_lidar->config.imu_rate),
                                             LivoxLidar::SetImuRatePushFrequencyCb, g_pLidar);
                    p_lidar->config.set_bits |= kConfigImuRate;
                }

                if (kDeviceTypeLidarAvia == info->type)
                {
                    LidarSetScanPattern(handle, (LidarScanPattern)(p_lidar->config.scan_pattern),
                                        LivoxLidar::SetScanPatternCb, g_pLidar);
                    p_lidar->config.set_bits |= kConfigScanPattern;
                }

                p_lidar->connect_state = kConnectStateConfig;
            }
        }
    }

    /** Query the firmware version of Livox LiDAR. */
    void LivoxLidar::DeviceInformationCb(livox_status status, uint8_t handle,
                                         DeviceInformationResponse *ack, void *client_data)
    {
        if (status != kStatusSuccess)
        {
            printf("Device Query Informations Failed : %d\n", status);
        }
        if (ack)
        {
            printf("firm ver: %d.%d.%d.%d\n",
                   ack->firmware_version[0],
                   ack->firmware_version[1],
                   ack->firmware_version[2],
                   ack->firmware_version[3]);
        }
    }

    /** Callback function of Lidar error message. */
    void LivoxLidar::LidarErrorStatusCb(livox_status status, uint8_t handle, ErrorMessage *message)
    {
        static uint32_t error_message_count = 0;
        if (message != NULL)
        {
            ++error_message_count;
            if (0 == (error_message_count % 100))
            {
                printf("handle: %u\n", handle);
                printf("temp_status : %u\n", message->lidar_error_code.temp_status);
                printf("volt_status : %u\n", message->lidar_error_code.volt_status);
                printf("motor_status : %u\n", message->lidar_error_code.motor_status);
                printf("dirty_warn : %u\n", message->lidar_error_code.dirty_warn);
                printf("firmware_err : %u\n", message->lidar_error_code.firmware_err);
                printf("pps_status : %u\n", message->lidar_error_code.device_status);
                printf("fan_status : %u\n", message->lidar_error_code.fan_status);
                printf("self_heating : %u\n", message->lidar_error_code.self_heating);
                printf("ptp_status : %u\n", message->lidar_error_code.ptp_status);
                printf("time_sync_status : %u\n", message->lidar_error_code.time_sync_status);
                printf("system_status : %u\n", message->lidar_error_code.system_status);
            }
        }
    }

    void LivoxLidar::ControlFanCb(livox_status status, uint8_t handle,
                                  uint8_t response, void *client_data)
    {
    }

    void LivoxLidar::SetPointCloudReturnModeCb(livox_status status, uint8_t handle,
                                               uint8_t response, void *client_data)
    {
        LivoxLidar *lds_lidar = static_cast<LivoxLidar *>(client_data);

        if (handle >= kMaxLidarCount)
        {
            return;
        }
        LidarDevice *p_lidar = &(lds_lidar->lidars_[handle]);

        if (status == kStatusSuccess)
        {
            p_lidar->config.set_bits &= ~((uint32_t)(kConfigReturnMode));
            printf("Set return mode success!\n");

            if (!p_lidar->config.set_bits)
            {
                LidarStartSampling(handle, LivoxLidar::StartSampleCb, lds_lidar);
                p_lidar->connect_state = kConnectStateSampling;
            }
        }
        else
        {
            LidarSetPointCloudReturnMode(handle, (PointCloudReturnMode)(p_lidar->config.return_mode),
                                         LivoxLidar::SetPointCloudReturnModeCb, lds_lidar);
            printf("Set return mode fail, try again!\n");
        }
    }

    void LivoxLidar::SetLidarModeCb(livox_status status, uint8_t handle,
                                    uint8_t response, void *client_data)
    {
        LivoxLidar *lds_lidar = static_cast<LivoxLidar *>(client_data);

        IF_(handle >= kMaxLidarCount);
        LidarDevice *p_lidar = &(lds_lidar->lidars_[handle]);

        IF_(status == kStatusSuccess);

        LidarSetMode(handle, p_lidar->config.lidar_mode, LivoxLidar::SetLidarModeCb, lds_lidar);
        printf("Set lidar mode fail, try again!\n");
    }

    void LivoxLidar::SetCoordinateCb(livox_status status, uint8_t handle,
                                     uint8_t response, void *client_data)
    {
        LivoxLidar *lds_lidar = static_cast<LivoxLidar *>(client_data);

        if (handle >= kMaxLidarCount)
        {
            return;
        }
        LidarDevice *p_lidar = &(lds_lidar->lidars_[handle]);

        if (status == kStatusSuccess)
        {
            p_lidar->config.set_bits &= ~((uint32_t)(kConfigCoordinate));
            printf("Set coordinate success!\n");

            if (!p_lidar->config.set_bits)
            {
                LidarStartSampling(handle, LivoxLidar::StartSampleCb, lds_lidar);
                p_lidar->connect_state = kConnectStateSampling;
            }
        }
        else
        {
            if (p_lidar->config.coordinate != 0)
            {
                SetSphericalCoordinate(handle, LivoxLidar::SetCoordinateCb, lds_lidar);
            }
            else
            {
                SetCartesianCoordinate(handle, LivoxLidar::SetCoordinateCb, lds_lidar);
            }

            printf("Set coordinate fail, try again!\n");
        }
    }

    void LivoxLidar::SetImuRatePushFrequencyCb(livox_status status, uint8_t handle,
                                               uint8_t response, void *client_data)
    {
        LivoxLidar *lds_lidar = static_cast<LivoxLidar *>(client_data);

        if (handle >= kMaxLidarCount)
        {
            return;
        }
        LidarDevice *p_lidar = &(lds_lidar->lidars_[handle]);

        if (status == kStatusSuccess)
        {
            p_lidar->config.set_bits &= ~((uint32_t)(kConfigImuRate));
            printf("Set imu rate success!\n");

            if (!p_lidar->config.set_bits)
            {
                LidarStartSampling(handle, LivoxLidar::StartSampleCb, lds_lidar);
                p_lidar->connect_state = kConnectStateSampling;
            }
        }
        else
        {
            LidarSetImuPushFrequency(handle, (ImuFreq)(p_lidar->config.imu_rate),
                                     LivoxLidar::SetImuRatePushFrequencyCb, lds_lidar);
            printf("Set imu rate fail, try again!\n");
        }
    }

    void LivoxLidar::SetScanPatternCb(livox_status status, uint8_t handle,
                                      DeviceParameterResponse *response, void *client_data)
    {
        LivoxLidar *lds_lidar = static_cast<LivoxLidar *>(client_data);

        if (handle >= kMaxLidarCount)
        {
            return;
        }
        LidarDevice *p_lidar = &(lds_lidar->lidars_[handle]);

        if (status == kStatusSuccess)
        {
            p_lidar->config.set_bits &= ~((uint32_t)(kConfigScanPattern));
            printf("Set scan pattern rate success!\n");

            if (!p_lidar->config.set_bits)
            {
                LidarStartSampling(handle, LivoxLidar::StartSampleCb, lds_lidar);
                p_lidar->connect_state = kConnectStateSampling;
            }
        }
        else
        {
            LidarSetScanPattern(handle, (LidarScanPattern)(p_lidar->config.scan_pattern),
                                LivoxLidar::SetScanPatternCb, lds_lidar);
            printf("Set scan pattern rate fail, try again!\n");
        }
    }

    /** Callback function of starting sampling. */
    void LivoxLidar::StartSampleCb(livox_status status, uint8_t handle,
                                   uint8_t response, void *client_data)
    {
        LivoxLidar *lds_lidar = static_cast<LivoxLidar *>(client_data);

        if (handle >= kMaxLidarCount)
        {
            return;
        }

        LidarDevice *p_lidar = &(lds_lidar->lidars_[handle]);
        if (status == kStatusSuccess)
        {
            if (response != 0)
            {
                p_lidar->connect_state = kConnectStateOn;
                printf("Lidar start sample fail : state[%d] handle[%d] res[%d]\n",
                       status, handle, response);
            }
            else
            {
                printf("Lidar start sample success\n");
            }
        }
        else if (status == kStatusTimeout)
        {
            p_lidar->connect_state = kConnectStateOn;
            printf("Lidar start sample timeout : state[%d] handle[%d] res[%d]\n",
                   status, handle, response);
        }
    }

    /** Callback function of stopping sampling. */
    void LivoxLidar::StopSampleCb(livox_status status, uint8_t handle,
                                  uint8_t response, void *client_data)
    {
    }

    /** Add broadcast code to whitelist */
    int LivoxLidar::AddBroadcastCodeToWhitelist(const char *bd_code)
    {
        if (!bd_code || (strlen(bd_code) > kBroadcastCodeSize) ||
            (whitelist_count_ >= kMaxLidarCount))
        {
            return -1;
        }

        if (LivoxLidar::FindInWhitelist(bd_code))
        {
            printf("%s is alrealy exist!\n", bd_code);
            return -1;
        }

        strcpy(broadcast_code_whitelist_[whitelist_count_], bd_code);
        ++whitelist_count_;

        return 0;
    }

    void LivoxLidar::AddLocalBroadcastCode(void)
    {
        for (size_t i = 0; i < sizeof(local_broadcast_code_list) / sizeof(intptr_t); ++i)
        {
            std::string invalid_bd = "000000000";
            printf("Local broadcast code : %s\n", local_broadcast_code_list[i]);
            if ((kBroadcastCodeSize == strlen(local_broadcast_code_list[i]) + 1) &&
                (nullptr == strstr(local_broadcast_code_list[i], invalid_bd.c_str())))
            {
                LivoxLidar::AddBroadcastCodeToWhitelist(local_broadcast_code_list[i]);
            }
            else
            {
                printf("Invalid local broadcast code : %s\n", local_broadcast_code_list[i]);
            }
        }
    }

    bool LivoxLidar::FindInWhitelist(const char *bd_code)
    {
        if (!bd_code)
        {
            return false;
        }

        for (uint32_t i = 0; i < whitelist_count_; i++)
        {
            if (strncmp(bd_code, broadcast_code_whitelist_[i], kBroadcastCodeSize) == 0)
            {
                return true;
            }
        }

        return false;
    }

}
#endif
