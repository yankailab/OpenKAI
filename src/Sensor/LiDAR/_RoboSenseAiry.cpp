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
        IF_F(!this->_PointCloud::loadConfig());
        const json &j = *m_pJ;

        DEL(m_pTdifop);
        m_pTdifop = createThread(jK(*m_pJ, "threadDIFOP"), "threadDIFOP");
        NULL_F(m_pTdifop);

        return true;
    }

    bool _RoboSenseAiry::saveConfig(bool bExport)
    {
        if (!_PointCloud::saveConfig(false))
        {
            return false;
        }


        if (m_pTdifop && !m_pTdifop->saveConfig(false))
        {
            return false;
        }

        if (!bExport)
        {
            return true;
        }
        return m_pJcfg->saveToFile();
    }

    bool _RoboSenseAiry::link(void)
    {
        IF_F(!this->_PointCloud::link());
        const json &j = *m_pJ;

        string n;

        n = "";
        jKv(j, "_UDPmsop", n);
        m_pUDPmsop = (_UDP *)(m_pM->findModule(n));
        NULL_F(m_pUDPmsop);

        n = "";
        jKv(j, "_UDPdifop", n);
        m_pUDPdifop = (_UDP *)(m_pM->findModule(n));
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
        NULL_F(m_pUDPmsop);
        NULL_F(m_pUDPdifop);

        return this->_PointCloud::check();
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
        if (nBr <= 0)
        {

            return false;
        }

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
        if (nBr <= 0)
        {

            return false;
        }

        // pDataRecv->version = pB[0];
        // memcpy(pDataRecv->data, &pB[36], LVX2_N_DATA);

        return true;
    }

    void _RoboSenseAiry::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_PointCloud::console(pConsole);

        _Console *pC = (_Console *)pConsole;

        //        pC->addMsg("States: " + i2str((int)m_lvxState));
    }

}
