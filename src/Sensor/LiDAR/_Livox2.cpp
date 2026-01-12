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

        // lvx state
        m_lvxState = lvxState_deviceQuery;
        m_lvxTout.setTout(USEC_10SEC);

        // lvx info
        m_lvxSN = "";
        memset(m_pLvxSN, 0, LVX2_N_SN);
        m_lvxIP = 0;
        m_lvxCmdPort = 0;
        m_lvxDevType = 0;

        // lvx default config
        m_lvxCfg.init();

        // lvx IMU
        m_tIMU = 0;
    }

    _Livox2::~_Livox2()
    {
    }

    int _Livox2::init(const json& j)
    {
        CHECK_(this->_PCstream::init(j));
        Kiss *pK = (Kiss *)pKiss;

        // lvx select
        = j.value("lvxSN", &m_lvxSN);
        if (!m_lvxSN.empty())
        {
            memcpy(m_pLvxSN, m_lvxSN.c_str(), m_lvxSN.length());
        }

        string ip;
        ip = "";
        = j.value("lvxIP", &ip);
        parseIP(ip.c_str(), (uint8_t *)&m_lvxIP);

        // lvx time out
        int tOutSec = 10;
        = j.value("tOutSec", &tOutSec);
        m_lvxTout.setTout(USEC_1SEC * tOutSec);

        // lvx config
        = j.value("lvxPCLdataType", &m_lvxCfg.m_pclDataType);
        = j.value("lvxPatternMode", &m_lvxCfg.m_patternMode);
        ip = "";
        = j.value("lvxHostIP", &ip);
        if (!parseIP(ip.c_str(), (uint8_t *)&m_lvxCfg.m_hostIP))
        {
            LOG_E("lvxHostIP parse failed");
            return OK_ERR_INVALID_VALUE;
        }
        = j.value("lvxHostPortState", &m_lvxCfg.m_hostPortState);
        = j.value("lvxHostPortPCL", &m_lvxCfg.m_hostPortPCL);
        = j.value("lvxHostPortIMU", &m_lvxCfg.m_hostPortIMU);
        = j.value("lvxFrameRate", &m_lvxCfg.m_frameRate);
        = j.value("lvxDetectMode", &m_lvxCfg.m_detectMode);
        = j.value("lvxWorkModeAfterBoot", &m_lvxCfg.m_workModeAfterBoot);
        = j.value("lvxWorkMode", &m_lvxCfg.m_workMode);
        = j.value("lvxIMUdataEn", &m_lvxCfg.m_imuDataEn);

        // common thread config
        Kiss *pKw;
        Kiss *pKr = pK->child("threadR");
        if (pKr->empty())
        {
            LOG_E("threadR not found");
            return OK_ERR_NOT_FOUND;
        }

        // Device Type Query
        m_pTdeviceQueryR = new _Thread();
        CHECK_d_l_(m_pTdeviceQueryR->init(pKr), DEL(m_pTdeviceQueryR), "TdeviceQueryR init failed");

        // Control Command
        pKw = nullptr;
        pKw = pK->child("threadCtrlCmdW");
        if (pKw->empty())
        {
            LOG_E("threadCtrlCmdW not found");
            return OK_ERR_NOT_FOUND;
        }
        m_pTctrlCmdW = new _Thread();
        CHECK_d_l_(m_pTctrlCmdW->init(pKw), DEL(m_pTctrlCmdW), "TctrlCmdW init failed");

        m_pTctrlCmdR = new _Thread();
        CHECK_d_l_(m_pTctrlCmdR->init(pKr), DEL(m_pTctrlCmdR), "TctrlCmdR init failed");

        // Push command
        m_pTpushCmdR = new _Thread();
        CHECK_d_l_(m_pTpushCmdR->init(pKr), DEL(m_pTpushCmdR), "TpushCmdR init failed");

        // Point Cloud Data
        m_pTpclR = new _Thread();
        CHECK_d_l_(m_pTpclR->init(pKr), DEL(m_pTpclR), "TpclR init failed");

        // IMU Data
        m_pTimuR = new _Thread();
        CHECK_d_l_(m_pTimuR->init(pKr), DEL(m_pTimuR), "TimuR init failed");

        return true;
    }

    int _Livox2::link(const json& j, ModuleMgr* pM)
    {
        CHECK_(this->_PCstream::link(j, pM));

        Kiss *pK = (Kiss *)m_pKiss;
        string n;

        n = "";
        = j.value("_UDPdeviceQuery", &n);
        m_pUDPdeviceQuery = (_UDP *)(pM->findModule(n));
        NULL__(m_pUDPdeviceQuery);

        n = "";
        = j.value("_UDPctrlCmd", &n);
        m_pUDPctrlCmd = (_UDP *)(pM->findModule(n));
        NULL__(m_pUDPctrlCmd);

        n = "";
        = j.value("_UDPpushCmd", &n);
        m_pUDPpushCmd = (_UDP *)(pM->findModule(n));
        NULL__(m_pUDPpushCmd);

        n = "";
        = j.value("_UDPpcl", &n);
        m_pUDPpcl = (_UDP *)(pM->findModule(n));
        NULL__(m_pUDPpcl);

        n = "";
        = j.value("_UDPimu", &n);
        m_pUDPimu = (_UDP *)(pM->findModule(n));
        NULL__(m_pUDPimu);

        // n = "";
        // = j.value("_IObaseLog", &n);
        // m_pUDPlog = (_IObase *)(pM->findModule(n));
        // NULL__(m_pUDPlog);

        return true;
    }

    int _Livox2::start(void)
    {
        NULL_F(m_pT);
        NULL__(m_pTdeviceQueryR);
        NULL__(m_pTctrlCmdW);
        NULL__(m_pTctrlCmdR);
        NULL__(m_pTpushCmdR);
        NULL__(m_pTpclR);
        NULL__(m_pTimuR);

        CHECK_(m_pT->start(getUpdateWdeviceQuery, this));
        CHECK_(m_pTdeviceQueryR->start(getUpdateRdeviceQuery, this));
        CHECK_(m_pTctrlCmdW->start(getUpdateWctrlCmd, this));
        CHECK_(m_pTctrlCmdR->start(getUpdateRctrlCmd, this));
        CHECK_(m_pTpushCmdR->start(getUpdateRpushCmd, this));
        CHECK_(m_pTpclR->start(getUpdateRpointCloud, this));
        CHECK_(m_pTimuR->start(getUpdateRimu, this));

        return true;
    }

    int _Livox2::check(void)
    {
        NULL__(m_pUDPdeviceQuery);
        NULL__(m_pUDPctrlCmd);
        NULL__(m_pUDPpushCmd);
        NULL__(m_pUDPpcl);
        NULL__(m_pUDPimu);
        //        NULL__(m_pUDPlog);

        return this->_PCstream::check();
    }

    // Common
    bool _Livox2::recvLivoxCmd(_IObase *pIO, LIVOX2_CMD *pCmdRecv, bool bParity)
    {
        NULL_F(pIO);
        NULL_F(pCmdRecv);

        int nBr = pIO->read((uint8_t *)pCmdRecv, sizeof(LIVOX2_CMD));
        if (nBr <= 0)
        {
            m_lvxState = lvxState_deviceQuery;
            return false;
        }

        IF_F(nBr < pCmdRecv->length);
        IF_F(pCmdRecv->sof != LVX2_SOF);

        if (bParity)
        {
            uint16_t crc16 = CRC::Calculate(pCmdRecv, 18, CRC::CRC_16_CCITTFALSE());
            IF_F(crc16 != pCmdRecv->crc16_h);

            uint32_t crc32 = CRC::Calculate(pCmdRecv->data, pCmdRecv->length - LVX2_CMD_N_HDR, CRC::CRC_32());
            IF_F(crc32 != pCmdRecv->crc32_d);
        }

        m_lvxTout.reStart();
        return true;
    }

    bool _Livox2::recvLivoxData(_IObase *pIO, LIVOX2_DATA *pDataRecv, bool bParity)
    {
        NULL_F(pIO);
        NULL_F(pDataRecv);

        uint8_t pB[LVX2_N_BUF];
        int nBr = pIO->read(pB, LVX2_N_BUF);
        if (nBr <= 0)
        {
            m_lvxState = lvxState_deviceQuery;
            return false;
        }

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

        m_lvxTout.reStart();
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

            if (m_lvxTout.bTout())
            {
                m_lvxState = lvxState_deviceQuery; // disconnected
            }
        }
    }

    void _Livox2::sendDeviceQuery(void)
    {
        IF_(!check());

        LIVOX2_CMD cmd;
        cmd.init(LVX2_CMD_DISCOVER, LVX2_CMD_REQ, 0);
        cmd.calcCRC();
        m_pUDPdeviceQuery->write((uint8_t *)&cmd, cmd.length);
    }

    void _Livox2::updateRdeviceQuery(void)
    {
        while (m_pTdeviceQueryR->bAlive())
        {
            LIVOX2_CMD cmd;
            if (recvLivoxCmd(m_pUDPdeviceQuery, &cmd))
            {
                handleDeviceQuery(cmd);
            }
        }
    }

    void _Livox2::handleDeviceQuery(const LIVOX2_CMD &cmd)
    {
        IF_(cmd.cmd_id != LVX2_CMD_DISCOVER);
        uint8_t rCode = cmd.data[0];
        IF_(rCode != LVX2_RET_SUCCESS);

        // check IP correspondence
        uint32_t lvxIP;
        memcpy((uint8_t *)&lvxIP, &cmd.data[18], 4);
        if (m_lvxIP != 0)
        {
            IF_(m_lvxIP != lvxIP);
        }

        // check SN correspondence if specified
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


        m_lvxCmdPort = *(uint16_t *)(&cmd.data[22]);
        //TODO: change cmd port in _UDP


        m_lvxDevType = cmd.data[1];

        m_lvxState = lvxState_init;
    }

    // Control Command
    void _Livox2::updateWctrlCmd(void)
    {
        while (m_pTctrlCmdW->bAlive())
        {
            m_pTctrlCmdW->autoFPS();

            if (m_lvxState == lvxState_init)
            {
                setLvxHost();
            }

            getLvxConfig();
        }
    }

    void _Livox2::getLvxConfig(void)
    {
        IF_(!check());

        LIVOX2_CMD cmd;
        cmd.init(LVX2_CMD_GET, LVX2_CMD_REQ, 0);

        // data
        cmd.addData((uint16_t)13); // key_num
        cmd.addData((uint16_t)0);  // rsvd

        // key value list
        cmd.addData((uint16_t)kKeyPclDataType);
        cmd.addData((uint16_t)kKeyPatternMode);
        cmd.addData((uint16_t)kKeyLidarIpCfg);
        cmd.addData((uint16_t)kKeyStateInfoHostIpCfg);
        cmd.addData((uint16_t)kKeyLidarPointDataHostIpCfg);
        cmd.addData((uint16_t)kKeyLidarImuHostIpCfg);
        cmd.addData((uint16_t)kKeyFrameRate);
        cmd.addData((uint16_t)kKeyDetectMode);
        cmd.addData((uint16_t)kKeyWorkModeAfterBoot);
        cmd.addData((uint16_t)kKeyWorkMode);
        cmd.addData((uint16_t)kKeyImuDataEn);
        cmd.addData((uint16_t)kKeyLidarDiagStatus);
        cmd.addData((uint16_t)kKeyHmsCode);

        cmd.calcCRC();
        m_pUDPctrlCmd->write((uint8_t *)&cmd, cmd.length);
    }

    void _Livox2::setLvxPCLdataType(void)
    {
        IF_(!check());

        LIVOX2_CMD cmd;
        cmd.init(LVX2_CMD_SET, LVX2_CMD_REQ, 0);
        // data
        cmd.addData((uint16_t)1); // key_num
        cmd.addData((uint16_t)0); // rsvd
        // key value list
        cmd.addData((uint16_t)kKeyPclDataType);
        cmd.addData((uint16_t)1);
        cmd.addData((uint8_t)m_lvxCfg.m_pclDataType);

        cmd.calcCRC();
        m_pUDPctrlCmd->write((uint8_t *)&cmd, cmd.length);
    }

    void _Livox2::setLvxPattern(void)
    {
        IF_(!check());

        LIVOX2_CMD cmd;
        cmd.init(LVX2_CMD_SET, LVX2_CMD_REQ, 0);
        // data
        cmd.addData((uint16_t)1); // key_num
        cmd.addData((uint16_t)0); // rsvd
        // key value list
        cmd.addData((uint16_t)kKeyPatternMode);
        cmd.addData((uint16_t)1);
        cmd.addData((uint8_t)m_lvxCfg.m_patternMode);

        cmd.calcCRC();
        m_pUDPctrlCmd->write((uint8_t *)&cmd, cmd.length);
    }

    void _Livox2::setLvxHost(void)
    {
        IF_(!check());

        LIVOX2_CMD cmd;
        cmd.init(LVX2_CMD_SET, LVX2_CMD_REQ, 0);
        // data
        cmd.addData((uint16_t)3); // key_num
        cmd.addData((uint16_t)0); // rsvd

        // state
        cmd.addData((uint16_t)kKeyStateInfoHostIpCfg); // key
        cmd.addData((uint16_t)8);                      // length
        cmd.addData(&m_lvxCfg.m_hostIP, 4);
        cmd.addData((uint16_t)m_lvxCfg.m_hostPortState);
        cmd.addData((uint16_t)0);

        // point cloud
        cmd.addData((uint16_t)kKeyLidarPointDataHostIpCfg);
        cmd.addData((uint16_t)8);
        cmd.addData(&m_lvxCfg.m_hostIP, 4);
        cmd.addData((uint16_t)m_lvxCfg.m_hostPortPCL);
        cmd.addData((uint16_t)0);

        // IMU
        cmd.addData((uint16_t)kKeyLidarImuHostIpCfg);
        cmd.addData((uint16_t)8);
        cmd.addData(&m_lvxCfg.m_hostIP, 4);
        cmd.addData((uint16_t)m_lvxCfg.m_hostPortIMU);
        cmd.addData((uint16_t)0);

        cmd.calcCRC();
        m_pUDPctrlCmd->write((uint8_t *)&cmd, cmd.length);
    }

    void _Livox2::setLvxFrameRate(void)
    {
        IF_(!check());

        LIVOX2_CMD cmd;
        cmd.init(LVX2_CMD_SET, LVX2_CMD_REQ, 0);
        // data
        cmd.addData((uint16_t)1); // key_num
        cmd.addData((uint16_t)0); // rsvd
        // key value list
        cmd.addData((uint16_t)kKeyFrameRate);
        cmd.addData((uint16_t)1);
        cmd.addData((uint8_t)m_lvxCfg.m_frameRate);

        cmd.calcCRC();
        m_pUDPctrlCmd->write((uint8_t *)&cmd, cmd.length);
    }

    void _Livox2::setLvxDetectMode(void)
    {
        IF_(!check());

        LIVOX2_CMD cmd;
        cmd.init(LVX2_CMD_SET, LVX2_CMD_REQ, 0);
        // data
        cmd.addData((uint16_t)1); // key_num
        cmd.addData((uint16_t)0); // rsvd
        // key value list
        cmd.addData((uint16_t)kKeyDetectMode);
        cmd.addData((uint16_t)1);
        cmd.addData((uint8_t)m_lvxCfg.m_detectMode);

        cmd.calcCRC();
        m_pUDPctrlCmd->write((uint8_t *)&cmd, cmd.length);
    }

    void _Livox2::setLvxWorkModeAfterBoot(void)
    {
        IF_(!check());

        LIVOX2_CMD cmd;
        cmd.init(LVX2_CMD_SET, LVX2_CMD_REQ, 0);
        // data
        cmd.addData((uint16_t)1); // key_num
        cmd.addData((uint16_t)0); // rsvd
        // key value list
        cmd.addData((uint16_t)kKeyWorkModeAfterBoot);
        cmd.addData((uint16_t)1);
        cmd.addData((uint8_t)m_lvxCfg.m_workModeAfterBoot);

        cmd.calcCRC();
        m_pUDPctrlCmd->write((uint8_t *)&cmd, cmd.length);
    }

    void _Livox2::setLvxWorkMode(void)
    {
        IF_(!check());

        LIVOX2_CMD cmd;
        cmd.init(LVX2_CMD_SET, LVX2_CMD_REQ, 0);
        // data
        cmd.addData((uint16_t)1); // key_num
        cmd.addData((uint16_t)0); // rsvd
        // key value list
        cmd.addData((uint16_t)kKeyWorkMode);
        cmd.addData((uint16_t)1);
        cmd.addData((uint8_t)m_lvxCfg.m_workMode);

        cmd.calcCRC();
        m_pUDPctrlCmd->write((uint8_t *)&cmd, cmd.length);
    }

    void _Livox2::setLvxIMUdataEn(void)
    {
        IF_(!check());

        LIVOX2_CMD cmd;
        cmd.init(LVX2_CMD_SET, LVX2_CMD_REQ, 0);
        // data
        cmd.addData((uint16_t)1); // key_num
        cmd.addData((uint16_t)0); // rsvd
        // key value list
        cmd.addData((uint16_t)kKeyImuDataEn);
        cmd.addData((uint16_t)1);
        cmd.addData((uint8_t)m_lvxCfg.m_imuDataEn);

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
        IF_(cmd.cmd_id != LVX2_CMD_GET);
        IF_(cmd.cmd_type != LVX2_CMD_ACK);

        uint8_t rCode = cmd.data[0];
        //        IF_(rCode != LVX2_RET_SUCCESS);
        uint16_t nK = *(uint16_t *)(&cmd.data[1]);
        uint8_t *pK = (uint8_t *)&cmd.data[3];

        int iD = 0;
        for (int iK = 0; iK < nK; iK++)
        {
            uint16_t key = *((uint16_t *)&pK[iD]);
            iD += 2;
            uint16_t nKb = *((uint16_t *)&pK[iD]);
            iD += 2;

            uint8_t v;
            uint16_t v2;
            uint32_t v4;
            switch (key)
            {
            case kKeyPclDataType:
                v = pK[iD];
                if (v != m_lvxCfg.m_pclDataType)
                    setLvxPCLdataType();
                break;
            case kKeyPatternMode:
                v = pK[iD];
                if (v != m_lvxCfg.m_patternMode)
                    setLvxPattern();
                break;
            case kKeyLidarIpCfg:
                v4 = *(uint32_t *)&pK[iD];
                if(v4 != m_lvxIP)
                {
                    setLvxHost();
                }
                else
                {
                    m_lvxState = lvxState_work;
                }
                break;
            case kKeyStateInfoHostIpCfg:
                v4 = *(uint32_t *)&pK[iD];
                v2 = *(uint16_t *)&pK[iD + 4];
                if(v4 != m_lvxCfg.m_hostIP || v2 != m_lvxCfg.m_hostPortState)
                    setLvxHost();
                break;
            case kKeyLidarPointDataHostIpCfg:
                v4 = *(uint32_t *)&pK[iD];
                v2 = *(uint16_t *)&pK[iD + 4];
                if(v4 != m_lvxCfg.m_hostIP || v2 != m_lvxCfg.m_hostPortPCL)
                    setLvxHost();
                break;
            case kKeyLidarImuHostIpCfg:
                v4 = *(uint32_t *)&pK[iD];
                v2 = *(uint16_t *)&pK[iD + 4];
                if(v4 != m_lvxCfg.m_hostIP || v2 != m_lvxCfg.m_hostPortIMU)
                    setLvxHost();
                break;
            case kKeyFrameRate:
                v = pK[iD];
                if (v != m_lvxCfg.m_frameRate)
                    setLvxFrameRate();
                break;
            case kKeyDetectMode:
                v = pK[iD];
                if (v != m_lvxCfg.m_detectMode)
                    setLvxDetectMode();
                break;
            case kKeyWorkModeAfterBoot:
                v = pK[iD];
                if (v != m_lvxCfg.m_workModeAfterBoot)
                    setLvxWorkModeAfterBoot();
                break;
            case kKeyWorkMode:
                v = pK[iD];
                if (v != m_lvxCfg.m_workMode)
                    setLvxWorkMode();
                break;
            case kKeyImuDataEn:
                v = pK[iD];
                if (v != m_lvxCfg.m_imuDataEn)
                    setLvxIMUdataEn();
                break;
            case kKeyLidarDiagStatus:
                v2 = *(uint16_t *)&pK[iD];
                if(v2 != 0)
                {
                    // lvx error
                }
                break;
            case kKeyHmsCode:
                v4 = *(uint32_t *)&pK[iD];
                if(v4 != 0)
                {
                    // lvx hms error
                }
                break;
            };

            iD += nKb;
        }

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
        uint64_t tStamp = getTbootUs();

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
        IF_(!m_lvxCfg.m_imuDataEn);

//        uint64_t tStamp = *((uint64_t *)d.timestamp);
        uint64_t tStamp = getTbootNs();
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

        // cancel yaw rot
        vR.x = 0;
        vR.y = 0;
        vR.z = -vR.z;
        updateTranslationMatrix(true, &vR);

        LOG_I("IMU, data_num:" + i2str(d.dot_num) + ", data_type:" + i2str(d.data_type) + ", length:" + i2str(d.length) + ", frame_counter:" + i2str(d.frame_cnt));
    }

    LVX2_CONFIG _Livox2::getConfig(void)
    {
        return m_lvxCfg;
    }

    void _Livox2::setConfig(const LVX2_CONFIG &cfg)
    {
        m_lvxCfg = cfg;
    }

    void _Livox2::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_PCstream::console(pConsole);

        _Console *pC = (_Console *)pConsole;

		pC->addMsg("States: " + i2str((int)m_lvxState));
		pC->addMsg("SN: " + m_lvxSN);

    }

}
