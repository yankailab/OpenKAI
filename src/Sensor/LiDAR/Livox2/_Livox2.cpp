/*
 *  Created on: Jan 19, 2024
 *      Author: yankai
 */
#include "_Livox2.h"

namespace kai
{

    _Livox2::_Livox2()
    {
        m_SN = "";
        m_bOpen = false;
        m_lidarMode = kLivoxLidarNormal;
    }

    _Livox2::~_Livox2()
    {
    }

    bool _Livox2::init(void *pKiss)
    {
        IF_F(!this->_PCstream::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("SN", &m_SN);
        pK->v("lidarMode", (int*)&m_lidarMode);

        return true;
    }

    bool _Livox2::open(void)
    {

        m_bOpen = true;

        LOG_I("open() success");
        return true;
    }

    void _Livox2::close(void)
    {
        LivoxLidarSdkUninit();
    }

    int _Livox2::check(void)
    {
        return this->_PCstream::check();
    }

    bool _Livox2::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    void _Livox2::update(void)
    {
        while (m_pT->bRun())
        {
            if (!m_bOpen)
            {
                if (!open())
                {
                    m_pT->sleepT(SEC_2_USEC);
                    continue;
                }
            }

            m_pT->autoFPSfrom();

            updateLidar();

            m_pT->autoFPSto();
        }
    }

    bool _Livox2::updateLidar(void)
    {
//        m_pL->setLidarMode(m_broadcastCode, (LidarMode)m_lidarMode);
        // if(m_lidarMode != kLidarModePowerSaving)
        // {
        //     m_pL->setScanPattern(m_broadcastCode, (LidarScanPattern)m_scanPattern);
        // }

        return true;
    }

    void _Livox2::setLidarMode(LivoxLidarWorkMode m)
    {
        m_lidarMode = m;
    }

    void _Livox2::startStream(void)
    {
        m_lidarMode = kLivoxLidarNormal;
    }

    void _Livox2::stopStream(void)
    {
        m_lidarMode = kLivoxLidarSleep;
    }

    void _Livox2::CbPointCloud(LivoxLidarEthernetPacket *pD)
    {
        NULL_(pD);
        LOG_I("CbPointCloud data_num: " + i2str(pD->dot_num) + ", data_type: " + i2str(pD->data_type) + ", length: " + i2str(pD->length) + ", frame_counter: " + i2str(pD->frame_cnt));
    }

    void _Livox2::CbImuData(LivoxLidarEthernetPacket *pD)
    {
        NULL_(pD);
        LOG_I("CbImuData, data_num:" + i2str(pD->dot_num) + ", data_type:" + i2str(pD->data_type) + ", length:" + i2str(pD->length) + ", frame_counter:" + i2str(pD->frame_cnt));
    }

    void _Livox2::CbWorkMode(livox_status status, LivoxLidarAsyncControlResponse *pR)
    {
        NULL_(pR);
        LOG_I("CbWorkMode, status:" + i2str(status) + ", ret_code:" + i2str(pR->ret_code) + ", error_key:" + i2str(pR->error_key));
    }

    void _Livox2::CbLidarInfoChange(const LivoxLidarInfo *pI)
    {
        NULL_(pI);

        LOG_I("LidarInfoChangeCallback Lidar IP: " + string(pI->lidar_ip) + " SN: " + string(pI->sn));
        //SetLivoxLidarWorkMode(handle, kLivoxLidarNormal, sCbWorkMode, this);
        // LivoxLidarStartLogger(handle, kLivoxLidarRealTimeLog, sCbLoggerStart, this);
        //  SetLivoxLidarDebugPointCloud(handle, true, sCbDebugPointCloud, this);
        //  sleep(10);
        //  SetLivoxLidarDebugPointCloud(handle, false, DebugPointCloudCallback, nullptr);
    }

    // void _Livox2::CbRecvData(LivoxEthPacket *pData, void *pLivox)
    // {
    //     NULL_(pData);
    //     NULL_(pLivox);

    //     _Livox2 *pL = (_Livox2 *)pLivox;
    //     uint8_t tStampType = pData->timestamp_type;
    //     uint64_t tStamp = *((uint64_t *)(pData->timestamp));
    //     // uint64_t tNow = getTbootMs();//pL->m_pT->getTfrom();

    //     if (pData->data_type == kCartesian)
    //     {
    //         pL->addP((LivoxRawPoint *)pData->data, tStamp);
    //     }
    //     else if (pData->data_type == kExtendCartesian)
    //     {
    //         pL->addP((LivoxExtendRawPoint *)pData->data, tStamp);
    //     }
    //     else if (pData->data_type == kDualExtendCartesian)
    //     {
    //         pL->addDualP((LivoxDualExtendRawPoint *)pData->data, tStamp);
    //     }
    //     else if (pData->data_type == kTripleExtendCartesian)
    //     {
    //         pL->addTripleP((LivoxTripleExtendRawPoint *)pData->data, tStamp);
    //     }
    //     else if (pData->data_type == kImu)
    //     {
    //         pL->updateIMU((LivoxImuPoint *)pData->data);
    //     }
    // }

    // void _Livox2::addP(LivoxRawPoint *pP, uint64_t &tStamp)
    // {
    //     Vector3d vP(pP->x, pP->y, pP->z);
    //     Vector3f vC(1, 1, 1);
    //     vC[0] = float(pP->reflectivity) * m_ovRef;
    //     add(vP, vC, tStamp);
    // }

    // void _Livox2::addP(LivoxExtendRawPoint *pP, uint64_t &tStamp)
    // {
    //     Vector3d vP(pP->x, pP->y, pP->z);
    //     Vector3f vC(1, 1, 1);
    //     vC[0] = float(pP->reflectivity) * m_ovRef;
    //     add(vP, vC, tStamp);
    // }

    // void _Livox2::addDualP(LivoxDualExtendRawPoint *pP, uint64_t &tStamp)
    // {
    //     Vector3d vP1(pP->x1, pP->y1, pP->z1);
    //     Vector3d vP2(pP->x2, pP->y2, pP->z2);
    //     Vector3f vC(1, 1, 1);
    //     vC[0] = float(pP->reflectivity1) * m_ovRef;
    //     add(vP1, vC, tStamp);
    //     vC[0] = float(pP->reflectivity2) * m_ovRef;
    //     add(vP2, vC, tStamp);
    // }

    // void _Livox2::addTripleP(LivoxTripleExtendRawPoint *pP, uint64_t &tStamp)
    // {
    //     Vector3d vP1(pP->x1, pP->y1, pP->z1);
    //     Vector3d vP2(pP->x2, pP->y2, pP->z2);
    //     Vector3d vP3(pP->x3, pP->y3, pP->z3);
    //     Vector3f vC(1, 1, 1);
    //     vC[0] = float(pP->reflectivity1) * m_ovRef;
    //     add(vP1, vC, tStamp);
    //     vC[0] = float(pP->reflectivity2) * m_ovRef;
    //     add(vP2, vC, tStamp);
    //     vC[0] = float(pP->reflectivity3) * m_ovRef;
    //     add(vP3, vC, tStamp);
    // }

    // void _Livox2::updateIMU(LivoxImuPoint *pD)
    // {
    //     // printf("gxyz=(%f, %f, %f) axyz=(%f, %f, %f)\n",
    //     //        pD->gyro_x,
    //     //        pD->gyro_y,
    //     //        pD->gyro_z,
    //     //        pD->acc_x,
    //     //        pD->acc_y,
    //     //        pD->acc_z);
    // }

    void _Livox2::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ModuleBase::console(pConsole);

        _Console *pC = (_Console *)pConsole;
        //		pC->addMsg("nState: " + i2str(m_vStates.size()), 0);
    }

}
