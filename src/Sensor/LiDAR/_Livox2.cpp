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
        m_pTctrlCmdW = nullptr;
        m_pTctrlCmdR = nullptr;
        m_pTpushCmdR = nullptr;
        m_pTpclR = nullptr;
        m_pTimuR = nullptr;

        m_pUDPdeviceQuery = nullptr;
        m_pUDPctrlCmd = nullptr;
        m_pUDPpushCmd = nullptr;
        m_pUDPpcl = nullptr;
        m_pUDPimu = nullptr;
        m_pUDPlog = nullptr;

        m_lvxSN = "";
        memset(m_pLvxSN, 0, LVX2_N_SN);
        m_lvxIP = 0;
        m_lvxCmdPort = 0;

        m_lvxState = lvxState_deviceQuery;
        m_lvxWorkMode = kLivoxLidarNormal;

        m_tIMU = 0;
        m_bEnableIMU = true;

        m_hostIP = 0;
        m_hostPortState = 0;
        m_hostPortPCL = 0;
        m_hostPortIMU = 0;
    }

    _Livox2::~_Livox2()
    {
    }

    int _Livox2::init(void *pKiss)
    {
        CHECK_(this->_PCstream::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("lvxSN", &m_lvxSN);
        if (!m_lvxSN.empty())
        {
            memcpy(m_pLvxSN, m_lvxSN.c_str(), m_lvxSN.length());
        }

        pK->v("lvxWorkMode", (int *)&m_lvxWorkMode);
        pK->v("bEnableIMU", &m_bEnableIMU);

        string ip;

        ip = "";
        pK->v("lvxIP", &ip);
        parseIP(ip.c_str(), (uint8_t *)&m_lvxIP);

        pK->v("hostIP", &ip);
        if (!parseIP(ip.c_str(), (uint8_t *)&m_hostIP))
        {
            LOG_E("hostIP parse failed");
            return OK_ERR_INVALID_VALUE;
        }

        pK->v("hostPortState", &m_hostPortState);
        pK->v("hostPortPCL", &m_hostPortPCL);
        pK->v("hostPortIMU", &m_hostPortIMU);

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
        pKt = pK->child("threadCtrlCmdW");
        if (pKt->empty())
        {
            LOG_E("threadCtrlCmdW not found");
            return OK_ERR_NOT_FOUND;
        }

        m_pTctrlCmdW = new _Thread();
        CHECK_d_l_(m_pTctrlCmdW->init(pKt), DEL(m_pTctrlCmdW), "TctrlCmdW init failed");

        pKt = pK->child("threadCtrlCmdR");
        if (pKt->empty())
        {
            LOG_E("threadCtrlCmdR not found");
            return OK_ERR_NOT_FOUND;
        }

        m_pTctrlCmdR = new _Thread();
        CHECK_d_l_(m_pTctrlCmdR->init(pKt), DEL(m_pTctrlCmdR), "TctrlCmdR init failed");

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
        pKt = pK->child("threadPclR");
        if (pKt->empty())
        {
            LOG_E("threadPclR not found");
            return OK_ERR_NOT_FOUND;
        }

        m_pTpclR = new _Thread();
        CHECK_d_l_(m_pTpclR->init(pKt), DEL(m_pTpclR), "TpclR init failed");

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
        pK->v("_UDPdeviceQuery", &n);
        m_pUDPdeviceQuery = (_UDP *)(pK->findModule(n));
        NULL__(m_pUDPdeviceQuery, OK_ERR_NOT_FOUND);

        n = "";
        pK->v("_UDPctrlCmd", &n);
        m_pUDPctrlCmd = (_UDP *)(pK->findModule(n));
        NULL__(m_pUDPctrlCmd, OK_ERR_NOT_FOUND);

        n = "";
        pK->v("_UDPpushCmd", &n);
        m_pUDPpushCmd = (_UDP *)(pK->findModule(n));
        NULL__(m_pUDPpushCmd, OK_ERR_NOT_FOUND);

        n = "";
        pK->v("_UDPpcl", &n);
        m_pUDPpcl = (_UDP *)(pK->findModule(n));
        NULL__(m_pUDPpcl, OK_ERR_NOT_FOUND);

        n = "";
        pK->v("_UDPimu", &n);
        m_pUDPimu = (_UDP *)(pK->findModule(n));
        NULL__(m_pUDPimu, OK_ERR_NOT_FOUND);

        // n = "";
        // pK->v("_IObaseLog", &n);
        // m_pUDPlog = (_IObase *)(pK->findModule(n));
        // NULL__(m_pUDPlog, OK_ERR_NOT_FOUND);

        return OK_OK;
    }

    int _Livox2::start(void)
    {
        NULL__(m_pT, OK_ERR_NULLPTR);
        NULL__(m_pTdeviceQueryR, OK_ERR_NULLPTR);
        NULL__(m_pTctrlCmdW, OK_ERR_NULLPTR);
        NULL__(m_pTctrlCmdR, OK_ERR_NULLPTR);
        NULL__(m_pTpushCmdR, OK_ERR_NULLPTR);
        NULL__(m_pTpclR, OK_ERR_NULLPTR);
        NULL__(m_pTimuR, OK_ERR_NULLPTR);

        CHECK_(m_pT->start(getUpdateWdeviceQuery, this));
        CHECK_(m_pTdeviceQueryR->start(getUpdateRdeviceQuery, this));
        CHECK_(m_pTctrlCmdW->start(getUpdateWctrlCmd, this));
        CHECK_(m_pTctrlCmdR->start(getUpdateRctrlCmd, this));
        CHECK_(m_pTpushCmdR->start(getUpdateRpushCmd, this));
        CHECK_(m_pTpclR->start(getUpdateRpointCloud, this));
        CHECK_(m_pTimuR->start(getUpdateRimu, this));

        return OK_OK;
    }

    int _Livox2::check(void)
    {
        NULL__(m_pUDPdeviceQuery, OK_ERR_NULLPTR);
        NULL__(m_pUDPctrlCmd, OK_ERR_NULLPTR);
        NULL__(m_pUDPpushCmd, OK_ERR_NULLPTR);
        NULL__(m_pUDPpcl, OK_ERR_NULLPTR);
        NULL__(m_pUDPimu, OK_ERR_NULLPTR);
        //        NULL__(m_pUDPlog, OK_ERR_NULLPTR);

        return this->_PCstream::check();
    }

    // Common
    bool _Livox2::recvLivoxCmd(_IObase *pIO, LIVOX2_CMD *pCmdRecv, bool bParity)
    {
        NULL_F(pIO);
        NULL_F(pCmdRecv);

        int nBr = pIO->read((uint8_t *)pCmdRecv, sizeof(LIVOX2_CMD));
        IF_F(nBr <= 0);
        IF_F(nBr < pCmdRecv->length);
        IF_F(pCmdRecv->sof != LVX2_SOF);

        if (bParity)
        {
            uint16_t crc16 = CRC::Calculate(pCmdRecv, 18, CRC::CRC_16_CCITTFALSE());
            IF_F(crc16 != pCmdRecv->crc16_h);

            uint32_t crc32 = CRC::Calculate(pCmdRecv->data, pCmdRecv->length - LVX2_CMD_N_HDR, CRC::CRC_32());
            IF_F(crc32 != pCmdRecv->crc32_d);
        }

        return true;
    }

    bool _Livox2::recvLivoxData(_IObase *pIO, LIVOX2_DATA *pDataRecv, bool bParity)
    {
        NULL_F(pIO);
        NULL_F(pDataRecv);

        uint8_t pB[LVX2_N_BUF];
        int nBr = pIO->read(pB, LVX2_N_BUF);
        IF_F(nBr <= 0);

        pDataRecv->version = pB[0];
        pDataRecv->length = *((uint16_t *)&pB[1]);
        pDataRecv->time_interval = *((uint16_t *)&pB[3]);
        pDataRecv->dot_num = *((uint16_t *)&pB[5]);
        pDataRecv->udp_cnt = *((uint16_t *)&pB[7]);
        pDataRecv->frame_cnt = pB[9];
        pDataRecv->data_type = pB[10];
        pDataRecv->time_type = pB[11];
        pDataRecv->crc32 = *((uint32_t *)&pB[24]);
        IF_F(nBr < pDataRecv->length);

        if (bParity)
        {
            uint32_t crc32 = CRC::Calculate(&pB[28], pDataRecv->length - LVX2_DATA_N_HDR, CRC::CRC_32());
            IF_F(crc32 != pDataRecv->crc32);
        }

        memcpy(pDataRecv->data, &pB[36], LVX2_N_DATA);

        return true;
    }

    // Device Type Query
    void _Livox2::updateWdeviceQuery(void)
    {
        while (m_pT->bAlive())
        {
            m_pT->autoFPS();

            if (m_lvxState == lvxState_deviceQuery)
            {
                sendDeviceQuery();
            }
        }
    }

    void _Livox2::sendDeviceQuery(void)
    {
        IF_(check() != OK_OK);

        LIVOX2_CMD cmd;
        cmd.init(0x0000, 0x00, 0);
        cmd.calcCRC();
        m_pUDPdeviceQuery->write((uint8_t *)&cmd, cmd.length);
    }

    void _Livox2::updateRdeviceQuery(void)
    {
        while (m_pTdeviceQueryR->bAlive())
        {
            m_pTdeviceQueryR->autoFPS();

            LIVOX2_CMD cmd;
            if (recvLivoxCmd(m_pUDPdeviceQuery, &cmd))
            {
                handleDeviceQuery(cmd);
            }
        }
    }

    void _Livox2::handleDeviceQuery(const LIVOX2_CMD &cmd)
    {
        IF_(cmd.cmd_id != 0x0000);
        uint8_t rCode = cmd.data[0];
        IF_(rCode != 0x00); // LVX_RET_SUCCESS

        uint8_t pSN[LVX2_N_SN];
        memcpy(pSN, &cmd.data[2], LVX2_N_SN);
        if (m_lvxSN.empty())
        {
            memcpy(m_pLvxSN, pSN, LVX2_N_SN);
        }
        else
        {
            IF_(!bEqual(m_pLvxSN, pSN, LVX2_N_SN));
        }

        uint64_t lvxIP;
        memcpy((uint8_t *)&lvxIP, &cmd.data[18], 4);
        if (m_lvxIP != 0)
        {
            IF_(m_lvxIP != lvxIP);
        }

        m_lvxCmdPort = *(uint16_t *)(&cmd.data[22]);
        m_lvxDevType = cmd.data[1];

        m_lvxState = lvxState_config;
    }

    // Control Command
    void _Livox2::updateWctrlCmd(void)
    {
        while (m_pTctrlCmdW->bAlive())
        {
            m_pTctrlCmdW->autoFPS();

            if (m_lvxState >= lvxState_config)
            {
                updateCtrlCmd();
            }
        }
    }

    void _Livox2::updateCtrlCmd(void)
    {
        if (m_lvxState == lvxState_config)
        {
            setLvxHost();
            setLvxWorkMode();
            setLvxPattern();
            setLvxPCLdataType();

            m_lvxState = lvxState_work;
            return;
        }
    }

    void _Livox2::setLvxHost(void)
    {
        IF_(check() != OK_OK);

        LIVOX2_CMD cmd;
        cmd.init(0x0100, 0x00, 0);
        // data
        cmd.addData((uint16_t)3); // key_num
        cmd.addData((uint16_t)0); // rsvd

        // state
        cmd.addData((uint16_t)kKeyStateInfoHostIpCfg); // key
        cmd.addData((uint16_t)8);                      // length
        cmd.addData(&m_hostIP, 4);
        cmd.addData((uint16_t)m_hostPortState);
        cmd.addData((uint16_t)0);

        // point cloud
        cmd.addData((uint16_t)kKeyLidarPointDataHostIpCfg);
        cmd.addData((uint16_t)8);
        cmd.addData(&m_hostIP, 4);
        cmd.addData((uint16_t)m_hostPortPCL);
        cmd.addData((uint16_t)0);

        // IMU
        cmd.addData((uint16_t)kKeyLidarImuHostIpCfg);
        cmd.addData((uint16_t)8);
        cmd.addData(&m_hostIP, 4);
        cmd.addData((uint16_t)m_hostPortIMU);
        cmd.addData((uint16_t)0);

        cmd.calcCRC();
        m_pUDPctrlCmd->write((uint8_t *)&cmd, cmd.length);
    }

    void _Livox2::setLvxWorkMode(void)
    {
        IF_(check() != OK_OK);

        LIVOX2_CMD cmd;
        cmd.init(0x0100, 0x00, 0);
        // data
        cmd.addData((uint16_t)1); // key_num
        cmd.addData((uint16_t)0); // rsvd
        // key value list
        cmd.addData((uint16_t)kKeyWorkMode);
        cmd.addData((uint16_t)1);
        cmd.addData((uint8_t)m_lvxWorkMode);

        cmd.calcCRC();
        m_pUDPctrlCmd->write((uint8_t *)&cmd, cmd.length);
    }

    void _Livox2::setLvxPattern(void)
    {
        IF_(check() != OK_OK);

        LIVOX2_CMD cmd;
        cmd.init(0x0100, 0x00, 0);
        // data
        cmd.addData((uint16_t)1); // key_num
        cmd.addData((uint16_t)0); // rsvd
        // key value list
        cmd.addData((uint16_t)kKeyPatternMode);
        cmd.addData((uint16_t)1);
        cmd.addData((uint8_t)kLivoxLidarScanPatternNoneRepetive);

        cmd.calcCRC();
        m_pUDPctrlCmd->write((uint8_t *)&cmd, cmd.length);
    }

    void _Livox2::setLvxPCLdataType(void)
    {
        IF_(check() != OK_OK);

        LIVOX2_CMD cmd;
        cmd.init(0x0100, 0x00, 0);
        // data
        cmd.addData((uint16_t)1); // key_num
        cmd.addData((uint16_t)0); // rsvd
        // key value list
        cmd.addData((uint16_t)kKeyPclDataType);
        cmd.addData((uint16_t)1);
        cmd.addData((uint8_t)kLivoxLidarCartesianCoordinateHighData);

        cmd.calcCRC();
        m_pUDPctrlCmd->write((uint8_t *)&cmd, cmd.length);
    }

    void _Livox2::updateRctrlCmd(void)
    {
        while (m_pTctrlCmdR->bAlive())
        {
            LIVOX2_CMD cmd;
            if (recvLivoxCmd(m_pUDPctrlCmd, &cmd))
            {
                handleCtrlCmdAck(cmd);
            }
        }
    }

    void _Livox2::handleCtrlCmdAck(const LIVOX2_CMD &cmd)
    {
        IF_(cmd.cmd_id != 0x0000);
    }

    // Push command
    void _Livox2::updateRpushCmd(void)
    {
        while (m_pTpushCmdR->bAlive())
        {
            LIVOX2_CMD cmd;
            if (recvLivoxCmd(m_pUDPpushCmd, &cmd))
            {
                handlePushCmd(cmd);
            }
        }
    }

    void _Livox2::handlePushCmd(const LIVOX2_CMD &cmd)
    {
        IF_(cmd.cmd_id != 0x0000);
    }

    // Point Cloud Data
    void _Livox2::updateRpointCloud(void)
    {
        while (m_pTpclR->bAlive())
        {
            LIVOX2_DATA d;
            if (recvLivoxData(m_pUDPpcl, &d))
            {
                handlePointCloudData(d);
            }
        }
    }

    void _Livox2::handlePointCloudData(const LIVOX2_DATA &d)
    {
        // uint64_t tStamp = *((uint64_t *)(pD->timestamp));
        uint64_t tStamp = getApproxTbootUs();

        if (d.data_type == kLivoxLidarCartesianCoordinateHighData)
        {
            LivoxLidarCartesianHighRawPoint *pPd = (LivoxLidarCartesianHighRawPoint *)d.data;
            for (uint32_t i = 0; i < d.dot_num; i++)
            {
                LivoxLidarCartesianHighRawPoint *pP = &pPd[i];
                Vector3d vP(pP->x, pP->y, pP->z);
                vP *= 0.001;
                vP = m_A * vP;
                add(vP, Vector3f{m_vColorDefault.x, m_vColorDefault.y, m_vColorDefault.z}, tStamp);
            }
        }
        else if (d.data_type == kLivoxLidarCartesianCoordinateLowData)
        {
            LivoxLidarCartesianLowRawPoint *pP = (LivoxLidarCartesianLowRawPoint *)d.data;
        }
        else if (d.data_type == kLivoxLidarSphericalCoordinateData)
        {
            LivoxLidarSpherPoint *pP = (LivoxLidarSpherPoint *)d.data;
        }

        LOG_I("PCL data_num: " + i2str(d.dot_num) + ", data_type: " + i2str(d.data_type) + ", length: " + i2str(d.length) + ", frame_counter: " + i2str(d.frame_cnt));
    }

    // IMU
    void _Livox2::updateRimu(void)
    {
        while (m_pTimuR->bAlive())
        {
            LIVOX2_DATA d;
            if (recvLivoxData(m_pUDPimu, &d))
            {
                handleIMUdata(d);
            }
        }
    }

    void _Livox2::handleIMUdata(const LIVOX2_DATA &d)
    {
        IF_(!m_bEnableIMU);

        uint64_t tStamp = *((uint64_t *)d.timestamp);
        uint64_t dT = tStamp - m_tIMU;
        m_tIMU = tStamp;
        if (dT > USEC_1SEC * 1000)
            dT = 0;

        LivoxLidarImuRawPoint *pIMU = (LivoxLidarImuRawPoint *)d.data;

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

        LOG_I("IMU, data_num:" + i2str(d.dot_num) + ", data_type:" + i2str(d.data_type) + ", length:" + i2str(d.length) + ", frame_counter:" + i2str(d.frame_cnt));
    }

    void _Livox2::setLidarMode(LivoxLidarWorkMode m)
    {
        m_lvxWorkMode = m;
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
