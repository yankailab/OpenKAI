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

        jKv(j, "PCLframe", n);
        m_pPCL = dynamic_cast<PCLframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
        IF_Le_F(!m_pPCL, "PCLframe not found: " + n);

        n = "";
        jKv(j, "_UDPmsop", n);
        m_pUDPmsop = (_UDP *)(pM->findModule(n));
        NULL_F(m_pUDPmsop);

        n = "";
        jKv(j, "_UDPdifop", n);
        m_pUDPdifop = (_UDP *)(pM->findModule(n));
        NULL_F(m_pUDPdifop);

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
        NULL_F(m_pPCL);
        NULL_F(m_pUDPmsop);
        NULL_F(m_pUDPdifop);

        return this->_ReferenceFrame::check();
    }

    void _RoboSenseAiry::clear(void)
    {
        if (m_pPCL)
        {
            m_pPCL->set({});
        }
    }

    void _RoboSenseAiry::updateMSOP(void)
    {
        while (m_pT->bRun())
        {
            recvMSOP();
        }
    }

    bool _RoboSenseAiry::recvMSOP(void)
    {
        NULL_F(m_pUDPmsop);

        uint8_t pB[RS_MSOP_N];
        int nBr = m_pUDPmsop->read(pB, RS_MSOP_N);
        IF_F(nBr <= 0);

        // pDataRecv->version = pB[0];
        // memcpy(pDataRecv->data, &pB[36], LVX2_N_DATA);

        return true;
    }

    void _RoboSenseAiry::updateDIFOP(void)
    {
        while (m_pTdifop->bRun())
        {
            recvDIFOP();
        }
    }

    bool _RoboSenseAiry::recvDIFOP(void)
    {
        NULL_F(m_pUDPdifop);

        uint8_t pB[RS_MSOP_N];
        int nBr = m_pUDPdifop->read(pB, RS_MSOP_N);
        IF_F(nBr <= 0);

        // pDataRecv->version = pB[0];
        // memcpy(pDataRecv->data, &pB[36], LVX2_N_DATA);

        return true;
    }

    void _RoboSenseAiry::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ReferenceFrame::console(pConsole);

        _Console *pC = (_Console *)pConsole;

        //        pC->addMsg("States: " + i2str((int)m_lvxState));
    }

}
