/*
 *  Created on: Jan 19, 2024
 *      Author: yankai
 */
#include "_Livox2.h"

namespace kai
{

    _Livox2::_Livox2()
    {
        m_pLv = NULL;
        m_SN = "";
        m_handle = -1;
        m_bOpen = false;
        m_workMode = kLivoxLidarNormal;

        m_tIMU = 0;
    }

    _Livox2::~_Livox2()
    {
    }

    bool _Livox2::init(void *pKiss)
    {
        IF_F(!this->_PCstream::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("SN", &m_SN);
        pK->v("lidarMode", (int *)&m_workMode);

        //        FusionAhrsInitialise(&m_ahrs);

        return true;
    }

    bool _Livox2::link(void)
    {
        IF_F(!this->_PCstream::link());

        Kiss *pK = (Kiss *)m_pKiss;
        string n = "";

        pK->v("LivoxLidar2", &n);
        m_pLv = (LivoxLidar2 *)(pK->getInst(n));

        return true;
    }

    bool _Livox2::open(void)
    {
        NULL_F(m_pLv);

        LivoxLidar2device *pD = m_pLv->getDevice(m_SN);
        NULL_F(pD);
        m_handle = pD->m_handle;

        IF_F(!m_pLv->setCbData(m_handle, sCbPointCloud, (void *)this));
        IF_F(!m_pLv->setCbIMU(m_handle, sCbIMU, (void *)this));

        m_bOpen = true;
        LOG_I("open() success");
        return true;
    }

    void _Livox2::close(void)
    {
    }

    int _Livox2::check(void)
    {
        NULL__(m_pLv, -1);

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

    void _Livox2::updateLidar(void)
    {
        IF_(check() < 0);

        m_pLv->setWorkMode(m_handle, m_workMode);

        // if(m_workMode == kLivoxLidarNormal)
        //     m_pLv->setScanPattern(m_handle, m_scanPattern);
    }

    void _Livox2::setLidarMode(LivoxLidarWorkMode m)
    {
        m_workMode = m;
    }

    void _Livox2::startStream(void)
    {
        m_workMode = kLivoxLidarNormal;
    }

    void _Livox2::stopStream(void)
    {
        m_workMode = kLivoxLidarSleep;
    }

    void _Livox2::CbPointCloud(LivoxLidarEthernetPacket *pD)
    {
        NULL_(pD);
        LOG_I("CbPointCloud data_num: " + i2str(pD->dot_num) + ", data_type: " + i2str(pD->data_type) + ", length: " + i2str(pD->length) + ", frame_counter: " + i2str(pD->frame_cnt));

        uint8_t tStampType = pD->time_type;
        //        uint64_t tStamp = *((uint64_t *)(pD->timestamp));
        uint64_t tStamp = getApproxTbootUs();

        if (pD->data_type == kLivoxLidarCartesianCoordinateHighData)
        {
            LivoxLidarCartesianHighRawPoint *pPd = (LivoxLidarCartesianHighRawPoint *)pD->data;
            for (uint32_t i = 0; i < pD->dot_num; i++)
            {
                LivoxLidarCartesianHighRawPoint *pP = &pPd[i];
                Vector3d vP(pP->x, pP->y, pP->z);
                vP *= 0.001;
                vP = m_A * vP;
                add(vP, Vector3f{m_vDefaultColor.x, m_vDefaultColor.y, m_vDefaultColor.z}, tStamp);
            }
        }
        else if (pD->data_type == kLivoxLidarCartesianCoordinateLowData)
        {
            LivoxLidarCartesianLowRawPoint *pP = (LivoxLidarCartesianLowRawPoint *)pD->data;
        }
        else if (pD->data_type == kLivoxLidarSphericalCoordinateData)
        {
            LivoxLidarSpherPoint *pP = (LivoxLidarSpherPoint *)pD->data;
        }
    }

    void _Livox2::CbIMU(LivoxLidarEthernetPacket *pD)
    {
        NULL_(pD);
        LOG_I("CbIMU, data_num:" + i2str(pD->dot_num) + ", data_type:" + i2str(pD->data_type) + ", length:" + i2str(pD->length) + ", frame_counter:" + i2str(pD->frame_cnt));

        uint64_t tStamp = *((uint64_t *)pD->timestamp);
        uint64_t dT = tStamp - m_tIMU;
        m_tIMU = tStamp;
        if (dT > USEC_1SEC * 1000)
            dT = 0;

        LivoxLidarImuRawPoint *pIMU = (LivoxLidarImuRawPoint *)pD->data;

        m_SF.MahonyUpdate(
        // m_SF.MadgwickUpdate(
                          pIMU->gyro_x,
                          pIMU->gyro_y,
                          pIMU->gyro_z,
                          pIMU->acc_x,
                          pIMU->acc_y,
                          pIMU->acc_z,
                          ((float)dT)*1e-9);

        vDouble3 vR;
        vR.set(m_SF.getRoll(), m_SF.getPitch(), 0);//m_SF.getYaw());
        vR *= DEG_2_RAD;
        setRotation(vR);
        updateTranslationMatrix();
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
    }

    void _Livox2::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_PCstream::console(pConsole);

        _Console *pC = (_Console *)pConsole;
        //		pC->addMsg("nState: " + i2str(m_vStates.size()), 0);
    }

}
