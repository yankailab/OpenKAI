/*
 *  Created on: Jan 19, 2024
 *      Author: yankai
 */
#include "_Livox2.h"

namespace kai
{

    _Livox2::_Livox2()
    {
        m_pTdeviceQueryR = nullptr;
        m_pTcontrolCmdW = nullptr;
        m_pTcontrolCmdR = nullptr;
        m_pTpushCmdR = nullptr;
        m_pTpointCloudR = nullptr;
        m_pTimuR = nullptr;

        m_pDeviceQuery = nullptr;
        m_pCtrlCmd = nullptr;
        m_pPushCmd = nullptr;
        m_pPointCloud = nullptr;
        m_pIMU = nullptr;
        m_pLog = nullptr;

        m_state = livox2_deviceQuery;

        m_SN = "";
        m_workMode = kLivoxLidarNormal;

        m_tIMU = 0;
        m_bEnableIMU = true;
    }

    _Livox2::~_Livox2()
    {
    }

    int _Livox2::init(void *pKiss)
    {
        CHECK_(this->_PCstream::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("SN", &m_SN);
        pK->v("lidarMode", (int *)&m_workMode);
        pK->v("bEnableIMU", &m_bEnableIMU);

        Kiss *pKt;

        // Device Type Query
        pKt = pK->child("threadDeviceQueryR");
        if (pKt->empty())
        {
            LOG_E("threadDeviceQueryR not found");
            return OK_ERR_NOT_FOUND;
        }

        m_pTdeviceQueryR = new _Thread();
        CHECK_d_l_(m_pTdeviceQueryR->init(pKt), DEL(m_pTdeviceQueryR), "TdeviceQueryR init failed");

        // Control Command
        pKt = pK->child("threadControlCmdW");
        if (pKt->empty())
        {
            LOG_E("threadControlCmdW not found");
            return OK_ERR_NOT_FOUND;
        }

        m_pTcontrolCmdW = new _Thread();
        CHECK_d_l_(m_pTcontrolCmdW->init(pKt), DEL(m_pTcontrolCmdW), "TcontrolCmdW init failed");

        pKt = pK->child("threadControlCmdR");
        if (pKt->empty())
        {
            LOG_E("threadControlCmdR not found");
            return OK_ERR_NOT_FOUND;
        }

        m_pTcontrolCmdR = new _Thread();
        CHECK_d_l_(m_pTcontrolCmdR->init(pKt), DEL(m_pTcontrolCmdR), "TcontrolCmdR init failed");

        // Push command
        pKt = pK->child("threadPushCmdR");
        if (pKt->empty())
        {
            LOG_E("threadPushCmdR not found");
            return OK_ERR_NOT_FOUND;
        }

        m_pTpushCmdR = new _Thread();
        CHECK_d_l_(m_pTpushCmdR->init(pKt), DEL(m_pTpushCmdR), "TpushCmdR init failed");

        // Point Cloud Data
        pKt = pK->child("threadPointCloudR");
        if (pKt->empty())
        {
            LOG_E("threadPointCloudR not found");
            return OK_ERR_NOT_FOUND;
        }

        m_pTpointCloudR = new _Thread();
        CHECK_d_l_(m_pTpointCloudR->init(pKt), DEL(m_pTpointCloudR), "TpointCloudR init failed");

        // IMU Data
        pKt = pK->child("threadImuR");
        if (pKt->empty())
        {
            LOG_E("threadImuR not found");
            return OK_ERR_NOT_FOUND;
        }

        m_pTimuR = new _Thread();
        CHECK_d_l_(m_pTimuR->init(pKt), DEL(m_pTimuR), "TimuR init failed");

        return OK_OK;
    }

    int _Livox2::link(void)
    {
        CHECK_(this->_PCstream::link());

        Kiss *pK = (Kiss *)m_pKiss;
        string n;

        n = "";
        pK->v("_IObaseDeviceQuery", &n);
        m_pDeviceQuery = (_IObase *)(pK->findModule(n));
        NULL__(m_pDeviceQuery, OK_ERR_NOT_FOUND);

        n = "";
        pK->v("_IObaseCtrlCmd", &n);
        m_pCtrlCmd = (_IObase *)(pK->findModule(n));
        NULL__(m_pCtrlCmd, OK_ERR_NOT_FOUND);

        n = "";
        pK->v("_IObasePushCmd", &n);
        m_pPushCmd = (_IObase *)(pK->findModule(n));
        NULL__(m_pPushCmd, OK_ERR_NOT_FOUND);

        n = "";
        pK->v("_IObasePointCloud", &n);
        m_pPointCloud = (_IObase *)(pK->findModule(n));
        NULL__(m_pPointCloud, OK_ERR_NOT_FOUND);

        n = "";
        pK->v("_IObaseIMU", &n);
        m_pIMU = (_IObase *)(pK->findModule(n));
        NULL__(m_pIMU, OK_ERR_NOT_FOUND);

        // n = "";
        // pK->v("_IObaseLog", &n);
        // m_pLog = (_IObase *)(pK->findModule(n));
        // NULL__(m_pLog, OK_ERR_NOT_FOUND);

        return OK_OK;
    }

    int _Livox2::start(void)
    {
        NULL__(m_pT, OK_ERR_NULLPTR);
        NULL__(m_pTdeviceQueryR, OK_ERR_NULLPTR);
        NULL__(m_pTcontrolCmdW, OK_ERR_NULLPTR);
        NULL__(m_pTcontrolCmdR, OK_ERR_NULLPTR);
        NULL__(m_pTpushCmdR, OK_ERR_NULLPTR);
        NULL__(m_pTpointCloudR, OK_ERR_NULLPTR);
        NULL__(m_pTimuR, OK_ERR_NULLPTR);

        CHECK_(m_pT->start(getUpdateWdeviceQuery, this));
        CHECK_(m_pTdeviceQueryR->start(getUpdateRdeviceQuery, this));
        CHECK_(m_pTcontrolCmdW->start(getUpdateWctrlCmd, this));
        CHECK_(m_pTcontrolCmdR->start(getUpdateRctrlCmd, this));
        CHECK_(m_pTpushCmdR->start(getUpdateRpushCmd, this));
        CHECK_(m_pTpointCloudR->start(getUpdateRpointCloud, this));
        CHECK_(m_pTimuR->start(getUpdateRimu, this));

        return OK_OK;
    }

    int _Livox2::check(void)
    {
        NULL__(m_pDeviceQuery, OK_ERR_NULLPTR);
        NULL__(m_pCtrlCmd, OK_ERR_NULLPTR);
        NULL__(m_pPushCmd, OK_ERR_NULLPTR);
        NULL__(m_pPointCloud, OK_ERR_NULLPTR);
        NULL__(m_pIMU, OK_ERR_NULLPTR);
        //        NULL__(m_pLog, OK_ERR_NULLPTR);

        return this->_PCstream::check();
    }

    bool _Livox2::recvLivoxCmd(_IObase *pIO, LIVOX2_CMD *pCmdRecv)
    {
        NULL_F(pIO);
        NULL_F(pCmdRecv);

        int nBr = pIO->read((uint8_t *)pCmdRecv, LIVOX2_N_DATA);
        IF_F(nBr <= 0);
        IF_F(nBr < pCmdRecv->length);
        IF_F(pCmdRecv->sof != LIVOX2_SOF);

        return true;
    }

    bool _Livox2::recvLivoxData(_IObase *pIO, LIVOX2_DATA *pDataRecv)
    {
        NULL_F(pIO);
        NULL_F(pDataRecv);

        int nBr = pIO->read((uint8_t *)pDataRecv, LIVOX2_N_DATA);
        IF_F(nBr <= 0);
        IF_F(nBr < pDataRecv->length);

        return true;
    }

    void _Livox2::updateWdeviceQuery(void)
    {
        while (m_pT->bAlive())
        {
            m_pT->autoFPSfrom();

            if (m_state == livox2_deviceQuery)
            {
                sendDeviceQuery();
            }

            m_pT->autoFPSto();
        }
    }

    void _Livox2::sendDeviceQuery(void)
    {
        static uint32_t iSeq = 0;

        LivoxLidarCmdPacket p;

        p.sof = LIVOX2_SOF;
        p.version = 0;
        p.length = LIVOX2_CMD_N_HDR;
        p.seq_num = iSeq++;
        p.cmd_id = 0x0000;
        p.cmd_type = 0x00;    // REQ
        p.sender_type = 0x00; // Host computer
        p.crc16_h = CRC::Calculate(&p, 18, CRC::CRC_16_CCITTFALSE());
        p.crc32_d = 0; // CRC::Calculate(p.data, length, CRC::CRC_32());
        p.data[0] = 0;

        int nB = sizeof(p) - 1;
        m_pDeviceQuery->write((uint8_t *)&p, nB);
    }

    void _Livox2::updateRdeviceQuery(void)
    {
        while (m_pTdeviceQueryR->bAlive())
        {
            LIVOX2_CMD cmd;
            if (recvLivoxCmd(m_pDeviceQuery, &cmd))
            {
                handleDeviceQuery(cmd);
            }
        }
    }

    void _Livox2::handleDeviceQuery(const LIVOX2_CMD &cmd)
    {
        IF_(cmd.cmd_id != 0x0000);

        // TODO: crc check
    }

    void _Livox2::updateCtrlCmd(void)
    {
    }

    void _Livox2::updateWctrlCmd(void)
    {
        while (m_pTcontrolCmdW->bAlive())
        {
            m_pTcontrolCmdW->autoFPSfrom();

            m_pTcontrolCmdW->autoFPSto();
        }
    }

    void _Livox2::updateRctrlCmd(void)
    {
        while (m_pTcontrolCmdR->bAlive())
        {
            m_pTcontrolCmdR->autoFPSfrom();

            m_pTcontrolCmdR->autoFPSto();
        }
    }

    void _Livox2::updateRpushCmd(void)
    {
        while (m_pTpushCmdR->bAlive())
        {
            m_pTpushCmdR->autoFPSfrom();

            m_pTpushCmdR->autoFPSto();
        }
    }

    void _Livox2::updateRpointCloud(void)
    {
        while (m_pTpointCloudR->bAlive())
        {
            m_pTpointCloudR->autoFPSfrom();

            m_pTpointCloudR->autoFPSto();
        }
    }

    void _Livox2::updateRimu(void)
    {
        while (m_pTimuR->bAlive())
        {
            m_pTimuR->autoFPSfrom();

            m_pTimuR->autoFPSto();
        }
    }

    void _Livox2::setLidarMode(LivoxLidarWorkMode m)
    {
        m_workMode = m;
    }

    void _Livox2::recvPointCloud(LivoxLidarEthernetPacket *pD)
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
                add(vP, Vector3f{m_vColorDefault.x, m_vColorDefault.y, m_vColorDefault.z}, tStamp);
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

    void _Livox2::recvIMU(LivoxLidarEthernetPacket *pD)
    {
        NULL_(pD);
        IF_(!m_bEnableIMU);
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
            ((float)dT) * 1e-9);

        float *pQ = m_SF.getQuat();
        vDouble4 vQ(pQ[0], pQ[1], pQ[2], pQ[3]);
        setQuaternion(vQ);

        vDouble3 vR(m_SF.getRollRadians(), m_SF.getPitchRadians(), m_SF.getYawRadians());
        setRotation(vR);

        vR.x = 0;
        vR.y = 0;
        vR.z = -vR.z;
        updateTranslationMatrix(true, &vR);
    }

    void _Livox2::recvWorkMode(livox_status status, LivoxLidarAsyncControlResponse *pR)
    {
        NULL_(pR);
        LOG_I("CbWorkMode, status:" + i2str(status) + ", ret_code:" + i2str(pR->ret_code) + ", error_key:" + i2str(pR->error_key));
    }

    void _Livox2::recvLidarInfoChange(const LivoxLidarInfo *pI)
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



/*
    bool _Livox2::recvLivoxCmd(_IObase *pIO, LIVOX2_CMD_RECV *pCmdRecv)
    {
        NULL_F(pIO);
        NULL_F(pCmdRecv);

        uint8_t b;
        if (pCmdRecv->m_iB <= 0)
        {
            while (pIO->read(&b, 1) > 0)
            {
                IF_CONT(b != LIVOX2_SOF);

                ((uint8_t *)&pCmdRecv->m_cmd)[pCmdRecv->m_iB++] = b;
                break;
            }
            IF_F(pCmdRecv->m_iB <= 0);
        }

        int nBr;

        if (pCmdRecv->m_iB < LIVOX2_CMD_N_HDR)
        {
            while ((nBr = pIO->read((uint8_t *)&pCmdRecv->m_cmd.version, LIVOX2_CMD_N_HDR - pCmdRecv->m_iB)) > 0)
            {
                pCmdRecv->m_iB += nBr;

                if (pCmdRecv->m_iB >= LIVOX2_CMD_N_HDR)
                    break;
            }
            IF_F(pCmdRecv->m_iB < LIVOX2_CMD_N_HDR);
        }

        while ((nBr = pIO->read(&(((uint8_t *)&pCmdRecv->m_cmd)[pCmdRecv->m_iB]), pCmdRecv->m_cmd.length - pCmdRecv->m_iB)) > 0)
        {
            pCmdRecv->m_iB += nBr;

            if (pCmdRecv->m_iB >= pCmdRecv->m_cmd.length)
                break;
        }
        IF_F(pCmdRecv->m_iB < pCmdRecv->m_cmd.length);

        return true;
    }
*/
