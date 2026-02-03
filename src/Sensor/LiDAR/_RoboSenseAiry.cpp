/*
 *  Created on: Jan 19, 2024
 *      Author: yankai
 */
#include "_RoboSenseAiry.h"

namespace kai
{

    _RoboSenseAiry::_RoboSenseAiry()
    {
        m_pTdifop = nullptr;
        m_pUDPmsop = nullptr;
        m_pUDPdifop = nullptr;

        m_tIMU = 0;
    }

    _RoboSenseAiry::~_RoboSenseAiry()
    {
    }

    bool _RoboSenseAiry::init(const json &j)
    {
        IF_F(!this->_PCstream::init(j));

        DEL(m_pTdifop);
        m_pTdifop = createThread(jK(j, "threadDIFOP"), "threadDIFOP");
        NULL_F(m_pTdifop);

        return true;
    }

    bool _RoboSenseAiry::link(const json &j, ModuleMgr *pM)
    {
        IF_F(!this->_PCstream::link(j, pM));

        string n;

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

        IF_F(!m_pT->start(getUpdateMSOP, this));
        IF_F(!m_pTdifop->start(getUpdateDIFOP, this));

        return true;
    }

    bool _RoboSenseAiry::check(void)
    {
        NULL_F(m_pUDPmsop);
        NULL_F(m_pUDPdifop);

        return this->_PCstream::check();
    }

    void _RoboSenseAiry::updateMSOP(void)
    {
        while (m_pT->bAlive())
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
        while (m_pTdifop->bAlive())
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
        this->_PCstream::console(pConsole);

        _Console *pC = (_Console *)pConsole;

        //        pC->addMsg("States: " + i2str((int)m_lvxState));
    }

}
