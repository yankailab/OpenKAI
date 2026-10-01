/*
 *  Created on: Jan 19, 2024
 *      Author: yankai
 */
#include "_RoboSenseAiry.h"

namespace kai
{

    _RoboSenseAiry::_RoboSenseAiry()
    {
    }

    _RoboSenseAiry::~_RoboSenseAiry()
    {
    }

    bool _RoboSenseAiry::loadConfig(void)
    {
        IF_F(!this->_ReferenceFrame::loadConfig());
        const json &j = *m_pJ;

        DEL(m_pTdifop);
        m_pTdifop = createThread(jK(*m_pJ, "threadDIFOP"), "threadDIFOP");
        NULL_F(m_pTdifop);

        return true;
    }

    bool _RoboSenseAiry::saveConfig(bool bExport)
    {
        IF_F(!_ReferenceFrame::saveConfig(false));


        IF_F(m_pTdifop && !m_pTdifop->saveConfig(false));

        IF__(!bExport, true);
        return m_pJcfg->saveToFile();
    }

    bool _RoboSenseAiry::link(InstanceMgr *pM)
    {
        IF_F(!this->_ReferenceFrame::link(pM));
        const json &j = *m_pJ;

        string n;

        jKv(j, "PCLframeOut", n);
        m_pPCLout = dynamic_cast<PCLframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
        IF_Le_F(!m_pPCLout, "PCLframeOut not found: " + n);

        n = "";
        jKv(j, "BytePacketStreamMsopIn", n);
        m_pBpStreamMsopIn = dynamic_cast<BytePacketStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
        NULL_F(m_pBpStreamMsopIn);

        n = "";
        jKv(j, "BytePacketStreamDifopIn", n);
        m_pBpStreamDifopIn = dynamic_cast<BytePacketStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
        NULL_F(m_pBpStreamDifopIn);

        return true;
    }

    bool _RoboSenseAiry::start(void)
    {
        NULL_F(m_pT);
        NULL_F(m_pTdifop);

        IF_F(!m_pT->startThread(getUpdateMSOP, this));
        IF_F(!m_pTdifop->startThread(getUpdateDIFOP, this));

        return true;
    }

    bool _RoboSenseAiry::check(void)
    {
        NULL_F(m_pPCLout);
        NULL_F(m_pBpStreamMsopIn);
        NULL_F(m_pBpStreamDifopIn);

        return this->_ReferenceFrame::check();
    }

    void _RoboSenseAiry::clear(void)
    {
        if (m_pPCLout)
        {
            m_pPCLout->set({});
        }
    }

    void _RoboSenseAiry::updateMSOP(void)
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPS();

            recvMSOP();
        }
    }

    bool _RoboSenseAiry::recvMSOP(void)
    {
        NULL_F(m_pBpStreamMsopIn);

        vector<BYTE_PACKET> vPacket;
        m_pBpStreamMsopIn->getPackets(vPacket, m_tLastBpStreamMsopIn);
        for (const BYTE_PACKET &packet : vPacket)
        {
            m_tLastBpStreamMsopIn = packet.m_tStamp;
        }

        return !vPacket.empty();
    }

    void _RoboSenseAiry::updateDIFOP(void)
    {
        while (m_pTdifop->bRun())
        {
            m_pTdifop->autoFPS();

            recvDIFOP();
        }
    }

    bool _RoboSenseAiry::recvDIFOP(void)
    {
        NULL_F(m_pBpStreamDifopIn);

        vector<BYTE_PACKET> vPacket;
        m_pBpStreamDifopIn->getPackets(vPacket, m_tLastBpStreamDifopIn);
        for (const BYTE_PACKET &packet : vPacket)
        {
            m_tLastBpStreamDifopIn = packet.m_tStamp;
        }

        return !vPacket.empty();
    }

    void _RoboSenseAiry::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ReferenceFrame::console(pConsole);

        _Console *pC = (_Console *)pConsole;

        //        pC->addMsg("States: " + i2str((int)m_lvxState));
    }

}
