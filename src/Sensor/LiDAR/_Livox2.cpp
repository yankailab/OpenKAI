/*
 *  Created on: Jan 19, 2024
 *      Author: yankai
 */
#include "_Livox2.h"

namespace kai
{

    _Livox2::_Livox2()
    {
        // lvx state
        m_lvxTout.setTout(NSEC_SEC * 10);

        // lvx info
        memset(m_pLvxSN, 0, LVX2_N_SN);

        // lvx default config
        m_lvxCfg.init();
    }

    _Livox2::~_Livox2()
    {
    }

    bool _Livox2::loadConfig(void)
    {
        IF_F(!this->_ReferenceFrame::loadConfig());
        const json &j = *m_pJ;

        // lvx select
        jKv(j, "lvxSN", m_lvxSN);
        if (!m_lvxSN.empty())
        {
            memcpy(m_pLvxSN, m_lvxSN.c_str(), m_lvxSN.length());
        }

        string ip;
        jKv(j, "lvxIP", ip);
        parseIP(ip.c_str(), (uint8_t *)&m_lvxIP);

        // lvx time out
        int tOutSec = 10;
        jKv(j, "tOutSec", tOutSec);
        m_lvxTout.setTout(NSEC_SEC * tOutSec);

        // lvx config
        jKv(j, "lvxPCLdataType", m_lvxCfg.m_pclDataType);
        jKv(j, "lvxPatternMode", m_lvxCfg.m_patternMode);
        jKv(j, "lvxHostIP", ip);
        if (!parseIP(ip.c_str(), (uint8_t *)&m_lvxCfg.m_hostIP))
        {
            LOG_E("lvxHostIP parse failed");
            return false;
        }
        jKv(j, "lvxHostPortState", m_lvxCfg.m_hostPortState);
        jKv(j, "lvxHostPortPCL", m_lvxCfg.m_hostPortPCL);
        jKv(j, "lvxHostPortIMU", m_lvxCfg.m_hostPortIMU);
        jKv(j, "lvxFrameRate", m_lvxCfg.m_frameRate);
        jKv(j, "lvxDetectMode", m_lvxCfg.m_detectMode);
        jKv(j, "lvxWorkModeAfterBoot", m_lvxCfg.m_workModeAfterBoot);
        jKv(j, "lvxWorkMode", m_lvxCfg.m_workMode);
        jKv(j, "lvxIMUdataEn", m_lvxCfg.m_imuDataEn);

        jKv(j, "bIMUstab", m_bIMUstab);
        jKv<float>(j, "vColorDefault", m_vColorDefault);
        jKv(j, "nMaxFramePoints", m_nMaxFramePoints);
        IF_Le_F(m_nMaxFramePoints <= 0, "Invalid nMaxFramePoints");
        m_vFramePoints.clear();
        m_vFramePoints.reserve(m_nMaxFramePoints);
        m_tPCLframe = 0;
        m_bPCLframe = false;

        // Device Type Query
        DEL(m_pTdeviceQueryR);
        m_pTdeviceQueryR = createThread(jK(*m_pJ, "TdeviceQueryR"), "TdeviceQueryR");
        NULL_F(m_pTdeviceQueryR);

        // Control Command
        DEL(m_pTctrlCmdW);
        m_pTctrlCmdW = createThread(jK(*m_pJ, "TctrlCmdW"), "TctrlCmdW");
        NULL_F(m_pTctrlCmdW);

        DEL(m_pTctrlCmdR);
        m_pTctrlCmdR = createThread(jK(*m_pJ, "TctrlCmdR"), "TctrlCmdR");
        NULL_F(m_pTctrlCmdR);

        // Push command
        DEL(m_pTpushCmdR);
        m_pTpushCmdR = createThread(jK(*m_pJ, "TpushCmdR"), "TpushCmdR");
        NULL_F(m_pTpushCmdR);

        // Point Cloud Data
        DEL(m_pTpclR);
        m_pTpclR = createThread(jK(*m_pJ, "TpclR"), "TpclR");
        NULL_F(m_pTpclR);

        // IMU Data
        DEL(m_pTimuR);
        m_pTimuR = createThread(jK(*m_pJ, "TimuR"), "TimuR");
        NULL_F(m_pTimuR);

        return true;
    }

    bool _Livox2::saveConfig(bool bExport)
    {
        IF_F(!_ReferenceFrame::saveConfig(false));

        json &j = *m_pJ;
        j["lvxSN"] = m_lvxSN;
        j["lvxPCLdataType"] = m_lvxCfg.m_pclDataType;
        j["lvxPatternMode"] = m_lvxCfg.m_patternMode;
        j["lvxHostPortState"] = m_lvxCfg.m_hostPortState;
        j["lvxHostPortPCL"] = m_lvxCfg.m_hostPortPCL;
        j["lvxHostPortIMU"] = m_lvxCfg.m_hostPortIMU;
        j["lvxFrameRate"] = m_lvxCfg.m_frameRate;
        j["lvxDetectMode"] = m_lvxCfg.m_detectMode;
        j["lvxWorkModeAfterBoot"] = m_lvxCfg.m_workModeAfterBoot;
        j["lvxWorkMode"] = m_lvxCfg.m_workMode;
        j["lvxIMUdataEn"] = m_lvxCfg.m_imuDataEn;
        j["bIMUstab"] = m_bIMUstab;
        j["vColorDefault"] = {m_vColorDefault.x(), m_vColorDefault.y(), m_vColorDefault.z()};
        j["nMaxFramePoints"] = m_nMaxFramePoints;
        const uint8_t *pIP = reinterpret_cast<const uint8_t *>(&m_lvxIP);
        j["lvxIP"] = std::to_string(pIP[0]) + "." + std::to_string(pIP[1]) + "." +
                     std::to_string(pIP[2]) + "." + std::to_string(pIP[3]);
        pIP = reinterpret_cast<const uint8_t *>(&m_lvxCfg.m_hostIP);
        j["lvxHostIP"] = std::to_string(pIP[0]) + "." + std::to_string(pIP[1]) + "." +
                         std::to_string(pIP[2]) + "." + std::to_string(pIP[3]);
        j["tOutSec"] = m_lvxTout.m_tOut / NSEC_SEC;

        IF_F(m_pTdeviceQueryR && !m_pTdeviceQueryR->saveConfig(false));

        IF_F(m_pTctrlCmdW && !m_pTctrlCmdW->saveConfig(false));

        IF_F(m_pTctrlCmdR && !m_pTctrlCmdR->saveConfig(false));

        IF_F(m_pTpushCmdR && !m_pTpushCmdR->saveConfig(false));

        IF_F(m_pTpclR && !m_pTpclR->saveConfig(false));

        IF_F(m_pTimuR && !m_pTimuR->saveConfig(false));

        IF__(!bExport, true);
        return m_pJcfg->saveToFile();
    }

    bool _Livox2::link(InstanceMgr *pM)
    {
        IF_F(!this->_ReferenceFrame::link(pM));
        const json &j = *m_pJ;

        string n;

        n = "";
        jKv(j, "PCLframe", n);
        m_pPCL = dynamic_cast<PCLframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
        IF_Le_F(!m_pPCL, "PCLframe not found: " + n);

        n = "";
        jKv(j, "IMUstream", n);
        m_pIMU = dynamic_cast<IMUstream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
        IF_Le_F(!n.empty() && !m_pIMU, "IMUstream not found: " + n);

        n = "";
        jKv(j, "_UDPdeviceQuery", n);
        m_pUDPdeviceQuery = (_UDP *)(pM->findModule(n));
        NULL_F(m_pUDPdeviceQuery);

        n = "";
        jKv(j, "_UDPctrlCmd", n);
        m_pUDPctrlCmd = (_UDP *)(pM->findModule(n));
        NULL_F(m_pUDPctrlCmd);

        n = "";
        jKv(j, "_UDPpushCmd", n);
        m_pUDPpushCmd = (_UDP *)(pM->findModule(n));
        NULL_F(m_pUDPpushCmd);

        n = "";
        jKv(j, "_UDPpcl", n);
        m_pUDPpcl = (_UDP *)(pM->findModule(n));
        NULL_F(m_pUDPpcl);

        n = "";
        jKv(j, "_UDPimu", n);
        m_pUDPimu = (_UDP *)(pM->findModule(n));
        NULL_F(m_pUDPimu);

        // n = "";
        // jKv(j,"_IObaseLog",n);
        // m_pUDPlog = (_IObase *)(pM->findModule(n));
        // NULL_F(m_pUDPlog);

        return true;
    }

    bool _Livox2::start(void)
    {
        NULL_F(m_pT);
        NULL_F(m_pTdeviceQueryR);
        NULL_F(m_pTctrlCmdW);
        NULL_F(m_pTctrlCmdR);
        NULL_F(m_pTpushCmdR);
        NULL_F(m_pTpclR);
        NULL_F(m_pTimuR);

        IF_F(!m_pT->startThread(getUpdateWdeviceQuery, this));
        IF_F(!m_pTdeviceQueryR->startThread(getUpdateRdeviceQuery, this));
        IF_F(!m_pTctrlCmdW->startThread(getUpdateWctrlCmd, this));
        IF_F(!m_pTctrlCmdR->startThread(getUpdateRctrlCmd, this));
        IF_F(!m_pTpushCmdR->startThread(getUpdateRpushCmd, this));
        IF_F(!m_pTpclR->startThread(getUpdateRpointCloud, this));
        IF_F(!m_pTimuR->startThread(getUpdateRimu, this));

        return true;
    }

    bool _Livox2::check(void)
    {
        NULL_F(m_pPCL);
        NULL_F(m_pUDPdeviceQuery);
        NULL_F(m_pUDPctrlCmd);
        NULL_F(m_pUDPpushCmd);
        NULL_F(m_pUDPpcl);
        NULL_F(m_pUDPimu);
        //        NULL_F(m_pUDPlog);

        return this->_ReferenceFrame::check();
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

        m_lvxTout.reStart(getTns());
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

        if (nBr < 36)
        {
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
        memcpy(pDataRecv->timestamp, &pB[28], 8);

        pDataRecv->crc32 = *((uint32_t *)&pB[24]);
        if (pDataRecv->length < 36 || pDataRecv->length > nBr)
        {
            return false;
        }

        if (bParity)
        {
            uint32_t crc32 = CRC::Calculate(&pB[28], pDataRecv->length - LVX2_DATA_N_HDR, CRC::CRC_32());
            IF_F(crc32 != pDataRecv->crc32);
        }

        memcpy(pDataRecv->data, &pB[36], pDataRecv->length - 36);

        m_lvxTout.reStart(getTns());
        return true;
    }

    // Device Type Query
    void _Livox2::updateWdeviceQuery(void)
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPS();

            if (m_lvxState == lvxState_deviceQuery)
            {
                sendDeviceQuery();
            }

            if (m_lvxTout.bTout(getTns()))
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
        while (m_pTdeviceQueryR->bRun())
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
        // TODO: change cmd port in _UDP

        m_lvxDevType = cmd.data[1];

        m_lvxState = lvxState_init;
    }

    // Control Command
    void _Livox2::updateWctrlCmd(void)
    {
        while (m_pTctrlCmdW->bRun())
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
        while (m_pTctrlCmdR->bRun())
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
                if (v4 != m_lvxIP)
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
                if (v4 != m_lvxCfg.m_hostIP || v2 != m_lvxCfg.m_hostPortState)
                    setLvxHost();
                break;
            case kKeyLidarPointDataHostIpCfg:
                v4 = *(uint32_t *)&pK[iD];
                v2 = *(uint16_t *)&pK[iD + 4];
                if (v4 != m_lvxCfg.m_hostIP || v2 != m_lvxCfg.m_hostPortPCL)
                    setLvxHost();
                break;
            case kKeyLidarImuHostIpCfg:
                v4 = *(uint32_t *)&pK[iD];
                v2 = *(uint16_t *)&pK[iD + 4];
                if (v4 != m_lvxCfg.m_hostIP || v2 != m_lvxCfg.m_hostPortIMU)
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
                if (v2 != 0)
                {
                    // lvx error
                }
                break;
            case kKeyHmsCode:
                v4 = *(uint32_t *)&pK[iD];
                if (v4 != 0)
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
        while (m_pTpushCmdR->bRun())
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
        while (m_pTpclR->bRun())
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
        if (!m_pPCL || d.data_type != kLivoxLidarCartesianCoordinateHighData ||
            d.dot_num == 0 || d.dot_num > LVX2_N_DATA / sizeof(LivoxLidarCartesianHighRawPoint) ||
            d.length < 36 + d.dot_num * sizeof(LivoxLidarCartesianHighRawPoint))
        {
            return;
        }

        uint64_t tStamp = 0;
        memcpy(&tStamp, d.timestamp, sizeof(tStamp));
        if (!tStamp)
        {
            return;
        }

        std::lock_guard<std::mutex> frameLock(m_frameMutex);
        if (m_bPCLframe && d.frame_cnt != m_iPCLframe)
        {
            m_pPCL->set(m_vFramePoints, m_tPCLframe);
            m_vFramePoints.clear();
            m_vFramePoints.reserve(m_nMaxFramePoints);
            m_bPCLframe = false;
        }
        if (!m_bPCLframe)
        {
            m_iPCLframe = d.frame_cnt;
            m_tPCLframe = tStamp;
            m_bPCLframe = true;
        }

        Isometry3f pose;
        {
            std::lock_guard<std::mutex> lock(m_poseMutex);
            pose = m_mPosef;
        }

        const uint64_t dT = uint64_t(d.time_interval) * 100 / d.dot_num;
        for (size_t i = 0; i < d.dot_num && m_vFramePoints.size() < size_t(m_nMaxFramePoints); ++i)
        {
            LivoxLidarCartesianHighRawPoint raw;
            memcpy(&raw, d.data + i * sizeof(raw), sizeof(raw));
            GEOMETRY_POINT point;
            point.m_vP = pose * (Vector3f(raw.x, raw.y, raw.z) * 0.001f);
            point.m_vC = m_vColorDefault;
            point.m_tStamp = tStamp + dT * i;
            m_vFramePoints.push_back(point);
        }
    }

    void _Livox2::clear(void)
    {
        std::lock_guard<std::mutex> lock(m_frameMutex);
        m_vFramePoints.clear();
        m_tPCLframe = 0;
        m_bPCLframe = false;
        if (m_pPCL)
        {
            m_pPCL->set({});
        }
    }

    // IMU
    void _Livox2::updateRimu(void)
    {
        while (m_pTimuR->bRun())
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
        if (!m_lvxCfg.m_imuDataEn || d.length < 36 + sizeof(LivoxLidarImuRawPoint))
        {
            return;
        }

        LivoxLidarImuRawPoint imu;
        memcpy(&imu, d.data, sizeof(imu));
        const LivoxLidarImuRawPoint *pIMU = &imu;
        uint64_t tStamp = 0;
        memcpy(&tStamp, d.timestamp, sizeof(tStamp));

        if (m_pIMU)
        {
            Vector3f vAcc = Vector3f(pIMU->acc_x, pIMU->acc_y, pIMU->acc_z);
            m_pIMU->set(IMUstream::Type::Acc, vAcc, tStamp);

            Vector3f vGyro = Vector3f(pIMU->gyro_x, pIMU->gyro_y, pIMU->gyro_z);
            m_pIMU->set(IMUstream::Type::Gyro, vGyro, tStamp);
        }

        IF_(!m_bIMUstab);

        uint64_t dT = tStamp - m_tIMU;
        m_tIMU = tStamp;
        if (dT > NSEC_SEC)
            dT = 0;

        m_SF.MahonyUpdate(
            // m_SF.MadgwickUpdate(
            pIMU->gyro_x,
            pIMU->gyro_y,
            pIMU->gyro_z,
            pIMU->acc_x,
            pIMU->acc_y,
            pIMU->acc_z,
            nsec2sec<float>(dT));

        // Stabilize roll and pitch while cancelling the IMU's yaw rotation.
        {
            std::lock_guard<std::mutex> lock(m_poseMutex);
            setAngles(m_SF.getRollRadians(), m_SF.getPitchRadians(), 0.0);
        }

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
        this->_ReferenceFrame::console(pConsole);

        _Console *pC = (_Console *)pConsole;

        pC->addMsg("States: " + i2str((int)m_lvxState));
        pC->addMsg("SN: " + m_lvxSN);
    }

}
