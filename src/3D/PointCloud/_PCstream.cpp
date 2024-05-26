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

        m_pP = NULL;
        m_nP = 256;
        m_iP = 0;
    }

    _PCstream::~_PCstream()
    {
        m_iP = 0;
        m_nP = 0;
        DEL(m_pP);
    }

    bool _PCstream::init(void *pKiss)
    {
        IF_F(!this->_GeometryBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

        pK->v("nP", &m_nP);
        IF_F(m_nP <= 0);

        return initBuffer();
    }

    bool _PCstream::initBuffer(void)
    {
        mutexLock();

        m_pP = new GEOMETRY_POINT[m_nP];
        NULL_F(m_pP);
        m_iP = 0;

        for (int i = 0; i < m_nP; i++)
            m_pP[i].clear();

        mutexUnlock();

        return true;
    }

    void _PCstream::clear(void)
    {
        mutexLock();

        for (int i = 0; i < m_nP; i++)
            m_pP[i].clear();

        m_iP = 0;

        mutexUnlock();
    }

    bool _PCstream::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    int _PCstream::check(void)
    {
        NULL__(m_pP, -1);

        return this->_GeometryBase::check();
    }

    void _PCstream::update(void)
    {
//        sleep(1); // temporal, waiting for the data to be written into shared memory

        while (m_pT->bAlive())
        {
            m_pT->autoFPSfrom();

            updatePCstream();

            m_pT->autoFPSto();
        }
    }

    void _PCstream::updatePCstream(void)
    {
        IF_(check() < 0);

        readSharedMem();
        writeSharedMem();
    }

    void _PCstream::addPCstream(void *p, const uint64_t& tExpire)
    {
        IF_(check() < 0);
        NULL_(p);
        _PCstream *pS = (_PCstream *)p;

        mutexLock();

        uint64_t tNow = getApproxTbootUs();

        for (int i = 0; i < pS->m_nP; i++)
        {
            GEOMETRY_POINT* pP = &pS->m_pP[i];
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

        int nPw = small<int>(m_nP, m_pSM->nB()/sizeof(GEOMETRY_POINT) );

		memcpy(m_pSM->p(), m_pP, nPw * sizeof(GEOMETRY_POINT));
    }

    void _PCstream::readSharedMem(void)
    {
        NULL_(m_pSM);
        IF_(!m_pSM->bOpen());
        IF_(m_pSM->bWriter());

//		memcpy(m_pP, m_pSM->p(), m_nP * sizeof(GEOMETRY_POINT));
        GEOMETRY_POINT* pSM = (GEOMETRY_POINT*)m_pSM->p();
        for(int i=0; i<m_nP; i++)
        {
            GEOMETRY_POINT p = pSM[i];
            Vector3d eV = m_A * v2e(p.m_vP).cast<double>();
            p.m_vP = e2v((Vector3f)eV.cast<float>());

            float bR = 0.1;
            float bG = 0.1;
            float d = p.m_vP.len();
            p.m_vC.x = constrain<float>(1.0 - d * bR, 0, 1);
            p.m_vC.y = constrain<float>(d * bG, 0, 1);
            p.m_vC.z = 0;

            m_pP[m_iP] = p;
            m_iP = iInc(m_iP, m_nP);
        }
    }

    void _PCstream::copyTo(PointCloud *pPC, const uint64_t& tExpire)
    {
        IF_(check() < 0);
        NULL_(pPC);

        uint64_t tNow = getApproxTbootUs();

        for (int i = 0; i < m_nP; i++)
        {
            GEOMETRY_POINT* pP = &m_pP[i];
            IF_CONT(bExpired(pP->m_tStamp, tExpire, tNow));

        	pPC->points_.push_back(v2e(pP->m_vP).cast<double>());
        	pPC->colors_.push_back(v2e(pP->m_vC).cast<double>());
        }
    }

    void _PCstream::add(const Vector3d &vP, const Vector3f &vC, const uint64_t& tStamp)
    {
        GEOMETRY_POINT *pP = &m_pP[m_iP];
        pP->m_vP = e2v((Vector3f)vP.cast<float>());
        pP->m_vC = e2v((Vector3f)vC.cast<float>());
        pP->m_tStamp = tStamp;

        m_iP = iInc(m_iP, m_nP);
    }

    GEOMETRY_POINT* _PCstream::get(int i)
    {
        IF_N(i >= m_nP);

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
