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

        m_pUDPdeviceQuery = nullptr;
        m_pUDPctrlCmd = nullptr;
        m_pUDPpushCmd = nullptr;
        m_pUDPpointCloud = nullptr;
        m_pUDPimu = nullptr;
        m_pUDPlog = nullptr;

        m_state = livox2_deviceQuery;

        m_SN = "";
        m_workMode = kLivoxLidarNormal;

        m_tIMU = 0;
        m_bEnableIMU = true;

        m_IPstate = "";
        m_IPpcl = "";
        m_IPimu = "";
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

        pK->v("IPstate", &m_IPstate);
        pK->v("IPpcl", &m_IPpcl);
        pK->v("IPimu", &m_IPimu);

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
        pK->v("_UDPpointCloud", &n);
        m_pUDPpointCloud = (_UDP *)(pK->findModule(n));
        NULL__(m_pUDPpointCloud, OK_ERR_NOT_FOUND);

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
        NULL__(m_pUDPdeviceQuery, OK_ERR_NULLPTR);
        NULL__(m_pUDPctrlCmd, OK_ERR_NULLPTR);
        NULL__(m_pUDPpushCmd, OK_ERR_NULLPTR);
        NULL__(m_pUDPpointCloud, OK_ERR_NULLPTR);
        NULL__(m_pUDPimu, OK_ERR_NULLPTR);
        //        NULL__(m_pUDPlog, OK_ERR_NULLPTR);

        return this->_PCstream::check();
    }


	// Common
    bool _Livox2::recvLivoxCmd(_IObase *pIO, LIVOX2_CMD *pCmdRecv)
    {
        NULL_F(pIO);
        NULL_F(pCmdRecv);

        int nBr = pIO->read((uint8_t *)pCmdRecv, sizeof(LIVOX2_CMD));
        IF_F(nBr <= 0);
        IF_F(nBr < pCmdRecv->length);
        IF_F(pCmdRecv->sof != LIVOX2_SOF);

        uint16_t crc16 = CRC::Calculate(pCmdRecv, 18, CRC::CRC_16_CCITTFALSE());
        IF_F(crc16 != pCmdRecv->crc16_h);

        uint32_t crc32 = CRC::Calculate(pCmdRecv->data, pCmdRecv->length - LIVOX2_CMD_N_HDR, CRC::CRC_32());
        IF_F(crc32 != pCmdRecv->crc32_d);

        return true;
    }

    bool _Livox2::recvLivoxData(_IObase *pIO, LIVOX2_DATA *pDataRecv)
    {
        NULL_F(pIO);
        NULL_F(pDataRecv);

        uint8_t pB[LIVOX2_N_BUF];
        int nBr = pIO->read(pB, LIVOX2_N_BUF);
//        int nBr = pIO->read((uint8_t *)pDataRecv, sizeof(LIVOX2_DATA));
        IF_F(nBr <= 0);

        pDataRecv->version = pB[0];
        pDataRecv->length = *((uint16_t*)&pB[1]);
        pDataRecv->time_interval = *((uint16_t*)&pB[3]);
        pDataRecv->dot_num = *((uint16_t*)&pB[5]);
        pDataRecv->udp_cnt = *((uint16_t*)&pB[7]);
        pDataRecv->frame_cnt = pB[9];
        pDataRecv->data_type = pB[10];
        pDataRecv->time_type = pB[11];
        pDataRecv->crc32 = *((uint32_t*)&pB[24]);

        IF_F(nBr < pDataRecv->length);
        // uint32_t crc32 = CRC::Calculate(&pB[28], pDataRecv->length - LIVOX2_DATA_N_HDR, CRC::CRC_32());
        // IF_F(crc32 != pDataRecv->crc32);

        memcpy(pDataRecv->data, &pB[36], LIVOX2_N_DATA);

        return true;
    }


	// Device Type Query
    void _Livox2::updateWdeviceQuery(void)
    {
        while (m_pT->bAlive())
        {
            m_pT->autoFPS();

            if (m_state == livox2_deviceQuery)
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

        uint8_t pSN[16];
        memcpy(pSN, &cmd.data[2], 16);

        uint8_t pIP[4];
        memcpy(pIP, &cmd.data[18], 4);

        uint8_t rCode = cmd.data[0];
        m_devType = cmd.data[1];
        m_cmdPort = *(uint16_t *)(&cmd.data[22]);
    }


	// Control Command
    void _Livox2::updateWctrlCmd(void)
    {
        while (m_pTcontrolCmdW->bAlive())
        {
            m_pTcontrolCmdW->autoFPS();

            updateCtrlCmd();
        }
    }

    void _Livox2::updateCtrlCmd(void)
    {
        setHostIPconfig();
    }

	void _Livox2::setHostIPconfig(void)
    {
        IF_(check() != OK_OK);

        LIVOX2_CMD cmd;
        cmd.init(0x0100, 0x00, 0);
        // data
        cmd.addData((uint16_t)3); // key_num
        cmd.addData((uint16_t)0); // rsvd

        // key value list
        uint8_t pIPport[8];
        pIPport[6] = 0;
        pIPport[7] = 0;

        // state
        cmd.addData((uint16_t)kKeyStateInfoHostIpCfg); // key
        cmd.addData((uint16_t)8); // length
        IF_(!parseIPport(m_IPstate.c_str(), pIPport, (uint16_t*)&pIPport[4]));
        cmd.addData(pIPport, 8);

        // point cloud
        cmd.addData((uint16_t)kKeyLidarPointDataHostIpCfg);
        cmd.addData((uint16_t)8);
        IF_(!parseIPport(m_IPpcl.c_str(), pIPport, (uint16_t*)&pIPport[4]));
        cmd.addData(pIPport, 8);

        //IMU
        cmd.addData((uint16_t)kKeyLidarImuHostIpCfg);
        cmd.addData((uint16_t)8);
        IF_(!parseIPport(m_IPimu.c_str(), pIPport, (uint16_t*)&pIPport[4]));
        cmd.addData(pIPport, 8);

        cmd.calcCRC();
        m_pUDPctrlCmd->write((uint8_t *)&cmd, cmd.length);
    }

    void _Livox2::setPclDataType(void)
    {
        IF_(check() != OK_OK);

        LIVOX2_CMD cmd;
        cmd.init(0x0100, 0x00, 0);
        // data
        cmd.addData((uint16_t)1); // key_num
        cmd.addData((uint16_t)0); // rsvd
        // key value list
        cmd.addData((uint16_t)kKeyPclDataType); // key
        cmd.addData((uint16_t)1);               // length
        cmd.addData((uint8_t)kLivoxLidarCartesianCoordinateHighData);

        cmd.calcCRC();
        m_pUDPctrlCmd->write((uint8_t *)&cmd, cmd.length);
    }

    void _Livox2::setPatternMode(void)
    {
        IF_(check() != OK_OK);

        LIVOX2_CMD cmd;
        cmd.init(0x0100, 0x00, 0);
        // data
        cmd.addData((uint16_t)1); // key_num
        cmd.addData((uint16_t)0); // rsvd
        // key value list
        cmd.addData((uint16_t)kKeyPatternMode); // key
        cmd.addData((uint16_t)1);               // length
        cmd.addData((uint8_t)kLivoxLidarScanPatternNoneRepetive);

        cmd.calcCRC();
        m_pUDPctrlCmd->write((uint8_t *)&cmd, cmd.length);
    }

    void _Livox2::updateRctrlCmd(void)
    {
        while (m_pTcontrolCmdR->bAlive())
        {
            m_pTcontrolCmdR->autoFPS();

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
            m_pTpushCmdR->autoFPS();

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
        while (m_pTpointCloudR->bAlive())
        {
//            m_pTpointCloudR->autoFPS();

            LIVOX2_DATA d;
            if (recvLivoxData(m_pUDPpointCloud, &d))
            {
                handlePointCloudData(d);
            }
        }
    }

    void _Livox2::handlePointCloudData(const LIVOX2_DATA &d)
    {
        LOG_I("CbPointCloud data_num: " + i2str(d.dot_num) + ", data_type: " + i2str(d.data_type) + ", length: " + i2str(d.length) + ", frame_counter: " + i2str(d.frame_cnt));

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
    }


	// IMU
    void _Livox2::updateRimu(void)
    {
        while (m_pTimuR->bAlive())
        {
            m_pTimuR->autoFPS();

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
        LOG_I("CbIMU, data_num:" + i2str(d.dot_num) + ", data_type:" + i2str(d.data_type) + ", length:" + i2str(d.length) + ", frame_counter:" + i2str(d.frame_cnt));

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
    }





    void _Livox2::setLidarMode(LivoxLidarWorkMode m)
    {
        m_workMode = m;
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
