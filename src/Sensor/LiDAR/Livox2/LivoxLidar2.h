/*
 * LivoxLidar2.h
 *
 *  Created on: Jan 19, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_Sensor_LiDAR_LivoxLidar2_H_
#define OpenKAI_src_Sensor_LiDAR_LivoxLidar2_H_

#include "../../../Base/BASE.h"
#include "../../../Module/Kiss.h"
#include "../../../UI/_Console.h"

#include "livox_lidar_def.h"
#include "livox_lidar_api.h"

namespace kai
{

    typedef void (*CbLivoxLidar2data)(LivoxLidarEthernetPacket *pD, void *pLivox2);
    typedef void (*CbLivoxLidar2imu)(LivoxLidarEthernetPacket *pD, void *pLivox2);
    struct LivoxLidar2device
    {
        uint32_t m_handle = 0;
        string m_SN = "";
        LivoxLidarWorkMode m_mode = kLivoxLidarNormal;
        // LidarConnectState connect_state;
        // LivoxDeviceInfo info;
        // bool enable_fan;
        // uint32_t return_mode;
        // uint32_t coordinate;
        // uint32_t imu_rate;
        // uint32_t scan_pattern;

        CbLivoxLidar2data m_pCbData = NULL;
        CbLivoxLidar2imu m_pCbIMU = NULL;
        void *m_pLivox2 = NULL;
    };

    class LivoxLidar2 : public BASE
    {
    public:
        LivoxLidar2();
        ~LivoxLidar2();

        virtual int init(void *pKiss);
        virtual void console(void *pConsole);

        int open(void);
        void close(void);

        void startStream(void);
        void stopStream(void);

        uint32_t getDeviceHandle(const string &SN);
        LivoxLidar2device *getDevice(const string &SN, bool bFirst = true);
        LivoxLidar2device *getDevice(uint32_t handle);

        bool setCbData(uint32_t handle, CbLivoxLidar2data pCb, void *pLivox2);
        bool setCbIMU(uint32_t handle, CbLivoxLidar2imu pCb, void *pLivox2);
        bool setWorkMode(uint32_t handle, LivoxLidarWorkMode m);
        bool setScanPattern(uint32_t handle, LivoxLidarScanPattern p);

        // Callbacks
        static void sCbPointCloud(uint32_t handle, const uint8_t dev_type, LivoxLidarEthernetPacket *data, void *client_data)
        {
            NULL_(client_data);
            ((LivoxLidar2 *)client_data)->CbPointCloud(handle, dev_type, data);
        }

        static void sCbImuData(uint32_t handle, const uint8_t dev_type, LivoxLidarEthernetPacket *data, void *client_data)
        {
            NULL_(client_data);
            ((LivoxLidar2 *)client_data)->CbIMU(handle, dev_type, data);
        }

        static void sCbWorkMode(livox_status status, uint32_t handle, LivoxLidarAsyncControlResponse *response, void *client_data)
        {
            NULL_(client_data);
            ((LivoxLidar2 *)client_data)->CbWorkMode(status, handle, response);
        }

        static void sCbLoggerStart(livox_status status, uint32_t handle, LivoxLidarLoggerResponse *response, void *client_data)
        {
            NULL_(client_data);
            ((LivoxLidar2 *)client_data)->CbLoggerStart(status, handle, response);
        }

        static void sCbDebugPointCloud(livox_status status, uint32_t handle, LivoxLidarLoggerResponse *response, void *client_data)
        {
            NULL_(client_data);
            ((LivoxLidar2 *)client_data)->CbDebugPointCloud(status, handle, response);
        }

        static void sCbLidarInfoChange(const uint32_t handle, const LivoxLidarInfo *info, void *client_data)
        {
            NULL_(client_data);
            ((LivoxLidar2 *)client_data)->CbLidarInfoChange(handle, info);
        }

        static void sCbLidarInfo(const uint32_t handle, const uint8_t dev_type, const char *info, void *client_data)
        {
            NULL_(client_data);
            ((LivoxLidar2 *)client_data)->CbLidarInfo(handle, dev_type, info);
        }

        static void sCbQueryInternalInfo(livox_status status,
                                         uint32_t handle,
                                         LivoxLidarDiagInternalInfoResponse *response,
                                         void *client_data)
        {
            if (response == nullptr)
            {
                return;
            }

            printf("QueryInternalInfoCallback, status:%u, handle:%i, ret_code:%u.\n",
                   status, handle, response->ret_code);
        }

    protected:
        void CbPointCloud(uint32_t handle, const uint8_t dev_type, LivoxLidarEthernetPacket *pD);
        void CbIMU(uint32_t handle, const uint8_t dev_type, LivoxLidarEthernetPacket *pD);
        void CbWorkMode(livox_status status, uint32_t handle, LivoxLidarAsyncControlResponse *pR);
        void CbLoggerStart(livox_status status, uint32_t handle, LivoxLidarLoggerResponse *pR);
        void CbDebugPointCloud(livox_status status, uint32_t handle, LivoxLidarLoggerResponse *pR);
        void CbLidarInfoChange(const uint32_t handle, const LivoxLidarInfo *pI);
        void CbLidarInfo(const uint32_t handle, const uint8_t dev_type, const char *pI);

    private:
        string m_fConfig;
        bool m_bOpen;

        vector<LivoxLidar2device> m_vDevice;
    };

}
#endif
