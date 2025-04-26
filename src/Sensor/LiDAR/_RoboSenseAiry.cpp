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

    int _RoboSenseAiry::init(void *pKiss)
    {
        CHECK_(this->_PCstream::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        // common thread config
        Kiss *pKd = pK->child("threadDIFOP");
        if (pKd->empty())
        {
            LOG_E("threadDIFOP not found");
            return OK_ERR_NOT_FOUND;
        }

        m_pTdifop = new _Thread();
        CHECK_d_l_(m_pTdifop->init(pKd), DEL(m_pTdifop), "Tdifop init failed");

        return OK_OK;
    }

    int _RoboSenseAiry::link(void)
    {
        CHECK_(this->_PCstream::link());

        Kiss *pK = (Kiss *)m_pKiss;
        string n;

        n = "";
        pK->v("_UDPmsop", &n);
        m_pUDPmsop = (_UDP *)(pK->findModule(n));
        NULL__(m_pUDPmsop, OK_ERR_NOT_FOUND);

        n = "";
        pK->v("_UDPdifop", &n);
        m_pUDPdifop = (_UDP *)(pK->findModule(n));
        NULL__(m_pUDPdifop, OK_ERR_NOT_FOUND);

        return OK_OK;
    }

    int _RoboSenseAiry::start(void)
    {
        NULL__(m_pT, OK_ERR_NULLPTR);
        NULL__(m_pTdifop, OK_ERR_NULLPTR);

        CHECK_(m_pT->start(getUpdateMSOP, this));
        CHECK_(m_pTdifop->start(getUpdateDIFOP, this));

        return OK_OK;
    }

    int _RoboSenseAiry::check(void)
    {
        NULL__(m_pUDPmsop, OK_ERR_NULLPTR);
        NULL__(m_pUDPdifop, OK_ERR_NULLPTR);

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
