/*
 * _PCio.cpp
 *
 *  Created on: Oct 8, 2020
 *      Author: yankai
 */

#include "_PCsend.h"

namespace kai
{

    _PCsend::_PCsend()
    {
        m_pIO = nullptr;
        m_iPsent = 0;
        m_pB = nullptr;
        m_nB = 256;
        m_tInt = 100000;
    }

    _PCsend::~_PCsend()
    {
        DEL(m_pB);
    }

    bool _PCsend::init(const json &j)
    {
        IF_F(!this->_GeometryBase::init(j));

        jVar(j, "tInt", m_tInt);
        jVar(j, "nB", m_nB);

        DEL(m_pB);
        m_pB = new uint8_t[m_nB];
        NULL_F(m_pB);

        return true;
    }

    bool _PCsend::link(const json &j, ModuleMgr *pM)
    {
        IF_F(!this->_GeometryBase::link(j, pM));

        string n = "";
        jVar(j, "_IObase", n);
        m_pIO = (_IObase *)(pM->findModule(n));
        IF_Le_F(!m_pIO, "_IObase not found: " + n);

        return true;
    }

    bool _PCsend::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    bool _PCsend::check(void)
    {
        NULL_F(m_pIO);
        IF_F(!m_pIO->bOpen());

        return this->_GeometryBase::check();
    }

    void _PCsend::update(void)
    {
        while (m_pT->bAlive())
        {
            m_pT->autoFPS();

            sendPC();
        }
    }

    void _PCsend::sendPC(void)
    {
        IF_(!check());
        //    IF_(m_iPsent == m_ring.m_iP);

        const double PC_SCALE = 1000;
        const int PC_DB = 2;
        m_pB[0] = PB_BEGIN;
        m_pB[1] = PC_STREAM;
        int iB = PC_N_HDR;

        //	while(m_iPsent != m_ring.m_iP)
        {
            Vector3d vP;
            Vector3d vC;
            //        IF_CONT(!m_ring.get(&vP, &vC, &m_iPsent));

            pack_int16(&m_pB[iB], (int16_t)(vP.x() * PC_SCALE), false);
            iB += PC_DB;
            pack_int16(&m_pB[iB], (int16_t)(vP.y() * PC_SCALE), false);
            iB += PC_DB;
            pack_int16(&m_pB[iB], (int16_t)(vP.z() * PC_SCALE), false);
            iB += PC_DB;

            pack_int16(&m_pB[iB], (int16_t)(vC.x() * PC_SCALE), false);
            iB += PC_DB;
            pack_int16(&m_pB[iB], (int16_t)(vC.y() * PC_SCALE), false);
            iB += PC_DB;
            pack_int16(&m_pB[iB], (int16_t)(vC.z() * PC_SCALE), false);
            iB += PC_DB;

            if (iB + PC_DB * 6 > m_nB)
            {
                pack_int16(&m_pB[2], (int16_t)(iB - PC_N_HDR), false);
                while (!m_pIO->write(m_pB, iB))
                    m_pT->sleepT(m_tInt);

                iB = PC_N_HDR;
            }
        }

        if (iB > PC_N_HDR)
        {
            pack_int16(&m_pB[2], (int16_t)(iB - PC_N_HDR), false);
            while (!m_pIO->write(m_pB, iB))
                m_pT->sleepT(m_tInt);
        }

        m_pT->sleepT(m_tInt);

        // frame sync
        m_pB[0] = PB_BEGIN;
        m_pB[1] = PC_FRAME_END;
        m_pB[2] = 0;
        m_pB[3] = 0;
        while (!m_pIO->write(m_pB, PC_N_HDR))
            m_pT->sleepT(m_tInt);
    }

}
