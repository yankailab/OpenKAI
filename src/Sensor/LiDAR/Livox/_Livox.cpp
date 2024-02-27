/*
 *  Created on: Aug 21, 2019
 *      Author: yankai
 */
#include "_Livox.h"

namespace kai
{

    _Livox::_Livox()
    {
        m_bOpen = false;
        m_pL = NULL;
        m_iTransformed = 0;
        m_lidarMode = kLidarModeNormal;
        m_scanPattern = LidarScanPattern::kNoneRepetitiveScanPattern;
    }

    _Livox::~_Livox()
    {
    }

    bool _Livox::init(void *pKiss)
    {
        IF_F(!this->_PCstream::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

        pK->v("broadcastCode", &m_broadcastCode);
        pK->v("lidarMode", &m_lidarMode);
        pK->v("scanPattern", &m_scanPattern);

//		m_livoxCtrl.m_vRz = {m_vRange.x, m_vRange.y};

        string n;
        n = "";
        pK->v("LivoxLidar", &n);
        m_pL = (LivoxLidar *)(pK->getInst(n));

        return true;
    }

    bool _Livox::open(void)
    {
        IF_F(check() < 0);

        IF_F(!m_pL->setDataCallback(m_broadcastCode, CbRecvData, (void *)this));

        m_bOpen = true;

        LOG_I("Init LivoxLidar success! Starting discovering Lidars\n");
        return true;
    }

    void _Livox::close(void)
    {
    }

    int _Livox::check(void)
    {
        NULL__(m_pL, -1);

        return this->_PCstream::check();
    }

    bool _Livox::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    void _Livox::update(void)
    {
        while (m_pT->bThread())
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

    bool _Livox::updateLidar(void)
    {
        m_pL->setLidarMode(m_broadcastCode, (LidarMode)m_lidarMode);        
        // if(m_lidarMode != kLidarModePowerSaving)
        // {
        //     m_pL->setScanPattern(m_broadcastCode, (LidarScanPattern)m_scanPattern);            
        // }

        return true;
    }

    void _Livox::setLidarMode(LidarMode m)
    {
        m_lidarMode = m;
    }

    void _Livox::setScanPattern(LidarScanPattern p)
    {
        m_scanPattern = p;        
    }

    void _Livox::startStream(void)
    {
        m_lidarMode = kLidarModeNormal;
    }

    void _Livox::stopStream(void)
    {
        m_lidarMode = kLidarModePowerSaving;
    }

    void _Livox::CbRecvData(LivoxEthPacket *pData, void *pLivox)
    {
        NULL_(pData);
        NULL_(pLivox);

        _Livox *pL = (_Livox *)pLivox;
        uint8_t tStampType = pData->timestamp_type;
        uint64_t tStamp = *((uint64_t *)(pData->timestamp));
        //uint64_t tNow = getTbootMs();//pL->m_pT->getTfrom();

        if (pData->data_type == kCartesian)
        {
            pL->addP((LivoxRawPoint *)pData->data, tStamp);
        }
        else if (pData->data_type == kExtendCartesian)
        {
            pL->addP((LivoxExtendRawPoint *)pData->data, tStamp);
        }
        else if (pData->data_type == kDualExtendCartesian)
        {
            pL->addDualP((LivoxDualExtendRawPoint *)pData->data, tStamp);
        }
        else if (pData->data_type == kTripleExtendCartesian)
        {
            pL->addTripleP((LivoxTripleExtendRawPoint *)pData->data, tStamp);
        }
        else if (pData->data_type == kImu)
        {
            pL->updateIMU((LivoxImuPoint *)pData->data);
        }
    }

    void _Livox::addP(LivoxRawPoint *pP, uint64_t &tStamp)
    {
        Vector3d vP(pP->x, pP->y, pP->z);
        Vector3f vC(1,1,1);
        vC[0] = float(pP->reflectivity) * m_ovRef;
        add(vP, vC, tStamp);
    }

    void _Livox::addP(LivoxExtendRawPoint *pP, uint64_t &tStamp)
    {
        Vector3d vP(pP->x, pP->y, pP->z);
        Vector3f vC(1,1,1);
        vC[0] = float(pP->reflectivity) * m_ovRef;
        add(vP, vC, tStamp);
    }

    void _Livox::addDualP(LivoxDualExtendRawPoint *pP, uint64_t &tStamp)
    {
        Vector3d vP1(pP->x1, pP->y1, pP->z1);
        Vector3d vP2(pP->x2, pP->y2, pP->z2);
        Vector3f vC(1,1,1);
        vC[0] = float(pP->reflectivity1) * m_ovRef;
        add(vP1, vC, tStamp);
        vC[0] = float(pP->reflectivity2) * m_ovRef;
        add(vP2, vC, tStamp);
    }

    void _Livox::addTripleP(LivoxTripleExtendRawPoint *pP, uint64_t &tStamp)
    {
        Vector3d vP1(pP->x1, pP->y1, pP->z1);
        Vector3d vP2(pP->x2, pP->y2, pP->z2);
        Vector3d vP3(pP->x3, pP->y3, pP->z3);
        Vector3f vC(1,1,1);
        vC[0] = float(pP->reflectivity1) * m_ovRef;
        add(vP1, vC, tStamp);
        vC[0] = float(pP->reflectivity2) * m_ovRef;
        add(vP2, vC, tStamp);
        vC[0] = float(pP->reflectivity3) * m_ovRef;
        add(vP3, vC, tStamp);
    }

    void _Livox::updateIMU(LivoxImuPoint *pD)
    {
        // printf("gxyz=(%f, %f, %f) axyz=(%f, %f, %f)\n",
        //        pD->gyro_x,
        //        pD->gyro_y,
        //        pD->gyro_z,
        //        pD->acc_x,
        //        pD->acc_y,
        //        pD->acc_z);
    }

}
