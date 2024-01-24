/*
 * _Livox2.h
 *
 *  Created on: Jan 19, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_Sensor_LiDAR__Livox2_H_
#define OpenKAI_src_Sensor_LiDAR__Livox2_H_

#include "../../../3D/PointCloud/_PCstream.h"
#include "LivoxLidar2.h"

namespace kai
{
    struct LivoxCtrl
    {
        vFloat2 m_vRz = {0.0, 500.0}; // z region
    };

    class _Livox2 : public _PCstream
    {
    public:
        _Livox2();
        ~_Livox2();

        virtual bool init(void *pKiss);
        virtual int check(void);
        virtual bool start(void);
        virtual void console(void *pConsole);

        virtual void startStream(void);
        virtual void stopStream(void);

        void setLidarMode(LivoxLidarWorkMode m);


        // Callbacks
        static void sCbPointCloud(LivoxLidarEthernetPacket *data, void *client_data)
        {
            NULL_(data);
            NULL_(client_data);

            ((_Livox2 *)client_data)->CbPointCloud(data);
        }

        static void sCbImuData(LivoxLidarEthernetPacket *data, void *client_data)
        {
            NULL_(data);
            NULL_(client_data);

            ((_Livox2 *)client_data)->CbImuData(data);
        }

        static void sCbWorkMode(livox_status status, LivoxLidarAsyncControlResponse *response, void *client_data)
        {
            NULL_(response);
            NULL_(client_data);

            ((_Livox2 *)client_data)->CbWorkMode(status, response);
        }

        static void sCbLidarInfoChange(const LivoxLidarInfo *info, void *client_data)
        {
            NULL_(info);
            NULL_(client_data);

            ((_Livox2 *)client_data)->CbLidarInfoChange(info);
        }

    protected:
        void CbPointCloud(LivoxLidarEthernetPacket *pD);
        void CbImuData(LivoxLidarEthernetPacket *pD);
        void CbWorkMode(livox_status status, LivoxLidarAsyncControlResponse *pR);
        void CbLidarInfoChange(const LivoxLidarInfo *pI);

        // void CbRecvData(LivoxEthPacket *pData, void *pLivox);
        // void addP(LivoxRawPoint *pLp, uint64_t &tStamp);
        // void addP(LivoxExtendRawPoint *pLp, uint64_t &tStamp);
        // void addDualP(LivoxDualExtendRawPoint *pLp, uint64_t &tStamp);
        // void addTripleP(LivoxTripleExtendRawPoint *pLp, uint64_t &tStamp);
        // void updateIMU(LivoxImuPoint *pLd);

        bool open(void);
        void close(void);
        bool updateLidar(void);
        void update(void);
        static void *getUpdate(void *This)
        {
            ((_Livox2 *)This)->update();
            return NULL;
        }

    public:
        string m_SN;
        int m_handle;
        bool m_bOpen;
        LivoxLidarWorkMode m_lidarMode;
    };

}
#endif
