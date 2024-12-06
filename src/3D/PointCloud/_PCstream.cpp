/*
 * _PCstream.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_PCstream.h"

namespace kai
{

    _PCstream::_PCstream()
    {
        m_type = pc_stream;

        m_pP = nullptr;
        m_nP = 256;
        m_iP = 0;
    }

    _PCstream::~_PCstream()
    {
        m_iP = 0;
        m_nP = 0;
        DEL(m_pP);
    }

    int _PCstream::init(void *pKiss)
    {
        CHECK_(this->_GeometryBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("nP", &m_nP);
        IF__(m_nP <= 0, OK_ERR_INVALID_VALUE);

        return initGrid();
    }

    int _PCstream::initGrid(void)
    {
        mutexLock();

        m_pP = new GEOMETRY_POINT[m_nP];
        if(!m_pP)
        {
            mutexUnlock();
            return OK_ERR_ALLOCATION;
        }
        m_iP = 0;

        for (int i = 0; i < m_nP; i++)
            m_pP[i].clear();

        mutexUnlock();

        return OK_OK;
    }

    void _PCstream::clear(void)
    {
        mutexLock();

        for (int i = 0; i < m_nP; i++)
            m_pP[i].clear();

        m_iP = 0;

        mutexUnlock();
    }

    int _PCstream::start(void)
    {
        NULL__(m_pT, OK_ERR_NULLPTR);
        return m_pT->start(getUpdate, this);
    }

    int _PCstream::check(void)
    {
        NULL__(m_pP, OK_ERR_NULLPTR);

        return this->_GeometryBase::check();
    }

    void _PCstream::update(void)
    {
        //        sleep(1); // temporal, waiting for the data to be written into shared memory

        while (m_pT->bAlive())
        {
            m_pT->autoFPS();

            updatePCstream();

            m_pT->autoFPSto();
        }
    }

    void _PCstream::updatePCstream(void)
    {
        IF_(check() != OK_OK);

        readSharedMem();
        writeSharedMem();
    }

    void _PCstream::addPCstream(void *p, const uint64_t &tExpire)
    {
        IF_(check() != OK_OK);
        NULL_(p);
        _PCstream *pS = (_PCstream *)p;

        mutexLock();

        uint64_t tNow = getApproxTbootUs();

        for (int i = 0; i < pS->m_nP; i++)
        {
            GEOMETRY_POINT *pP = &pS->m_pP[i];
            IF_CONT(bExpired(pP->m_tStamp, tExpire, tNow));

            m_pP[m_iP] = *pP;
            m_iP = iInc(m_iP, m_nP);
        }

        mutexUnlock();
    }

    void _PCstream::writeSharedMem(void)
    {
        NULL_(m_pSM);
        IF_(!m_pSM->bOpen());
        IF_(!m_pSM->bWriter());

        int nPw = small<int>(m_nP, m_pSM->nB() / sizeof(GEOMETRY_POINT));

        memcpy(m_pSM->p(), m_pP, nPw * sizeof(GEOMETRY_POINT));
    }

    void _PCstream::readSharedMem(void)
    {
        NULL_(m_pSM);
        IF_(!m_pSM->bOpen());
        IF_(m_pSM->bWriter());

        //		memcpy(m_pP, m_pSM->p(), m_nP * sizeof(GEOMETRY_POINT));
        GEOMETRY_POINT *pSM = (GEOMETRY_POINT *)m_pSM->p();
        for (int i = 0; i < m_nP; i++)
        {
            GEOMETRY_POINT p = pSM[i];
            Vector3d eV = m_A * v2e(p.m_vP).cast<double>();
            p.m_vP = e2v((Vector3f)eV.cast<float>());

            if (m_bColOverwrite)
            {
                float d = p.m_vP.len();
                p.m_vC.set(
                    1.0 - (m_vkColR.constrain(d) - m_vkColR.x) * m_vkColOv.x,
                    (m_vkColG.constrain(d) - m_vkColG.x) * m_vkColOv.y,
                    (m_vkColB.constrain(d) - m_vkColB.x) * m_vkColOv.z);
            }

            m_pP[m_iP] = p;
            m_iP = iInc(m_iP, m_nP);
        }
    }

    void _PCstream::copyTo(PointCloud *pPC, const uint64_t &tExpire)
    {
        IF_(check() != OK_OK);
        NULL_(pPC);

        uint64_t tNow = getApproxTbootUs();

        for (int i = 0; i < m_nP; i++)
        {
            GEOMETRY_POINT *pP = &m_pP[i];
            IF_CONT(bExpired(pP->m_tStamp, tExpire, tNow));

            pPC->points_.push_back(v2e(pP->m_vP).cast<double>());
            pPC->colors_.push_back(v2e(pP->m_vC).cast<double>());
        }
    }

    void _PCstream::add(const Vector3d &vP, const Vector3f &vC, const uint64_t &tStamp)
    {
        GEOMETRY_POINT *pP = &m_pP[m_iP];
        pP->m_vP = e2v((Vector3f)vP.cast<float>());
        pP->m_vC = e2v((Vector3f)vC.cast<float>());
        pP->m_tStamp = tStamp;

        m_iP = iInc(m_iP, m_nP);
    }

    GEOMETRY_POINT *_PCstream::get(int i)
    {
        IF__(i >= m_nP, nullptr);

        return &m_pP[i];
    }

    int _PCstream::nP(void)
    {
        return m_nP;
    }

    int _PCstream::iP(void)
    {
        return m_iP;
    }

}
