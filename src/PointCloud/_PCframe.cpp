/*
 * _PCframe.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifdef USE_OPEN3D
#include "_PCframe.h"
#include "_PCstream.h"
#include "_PClattice.h"

namespace kai
{

    _PCframe::_PCframe()
    {
        m_type = pc_frame;

        pthread_mutex_init(&m_mutexPC, NULL);
    }

    _PCframe::~_PCframe()
    {
        pthread_mutex_destroy(&m_mutexPC);
    }

    bool _PCframe::init(void *pKiss)
    {
        IF_F(!this->_PCbase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        //frame buf
        int nPCreserve = 0;
        pK->v("nPCreserve", &nPCreserve);
        if (nPCreserve > 0)
        {
            m_sPC.get()->points_.reserve(nPCreserve);
            m_sPC.get()->colors_.reserve(nPCreserve);
            m_sPC.next()->points_.reserve(nPCreserve);
            m_sPC.next()->colors_.reserve(nPCreserve);
        }

        return true;
    }

    int _PCframe::check(void)
    {
        return this->_PCbase::check();
    }

    void _PCframe::getPC(PointCloud *pPC)
    {
        NULL_(pPC);

        pthread_mutex_lock(&m_mutexPC);
        *pPC = *m_sPC.get();
        pthread_mutex_unlock(&m_mutexPC);
    }

    void _PCframe::updatePC(void)
    {
        m_sPC.next()->Transform(m_mT);

        pthread_mutex_lock(&m_mutexPC);
        m_sPC.swap();
        m_sPC.next()->points_.clear();
        m_sPC.next()->colors_.clear();
        m_sPC.next()->normals_.clear();
        pthread_mutex_unlock(&m_mutexPC);
    }

    void _PCframe::getStream(void *p)
    {
        NULL_(p);

        _PCstream *pS = (_PCstream *)p;
        PointCloud *pPC = m_sPC.next();
        uint64_t tFrom = m_pT->getTfrom() - m_pInCtx.m_dT;
        if (m_pInCtx.m_dT >= UINT64_MAX)
            tFrom = 0;

        for (int i = 0; i < pS->m_nP; i++)
        {
            PC_POINT *pP = &pS->m_pP[i];
            IF_CONT(pP->m_tStamp <= tFrom);

            Vector3f C = pP->m_vC;
            if (m_shade == pcShade_colOvrr)
            {
                C = m_vShadeCol.v3f();
            }
            else if (m_shade == pcShade_colPos)
            {
                C = m_vShadeCol.v3f();
                C[0] *= constrain<float>(1.0 - abs(pP->m_vP[2]) * m_rShadePosCol, 0, 1);
            }

            pPC->points_.push_back(pP->m_vP);
            pPC->colors_.push_back(C.cast<double>());
            m_nPread++;
        }
    }

    void _PCframe::getFrame(void *p)
    {
        NULL_(p);

        _PCframe *pF = (_PCframe *)p;
        PointCloud pc;
        pF->getPC(&pc);

        *m_sPC.next() += pc;
        m_nPread += pc.points_.size();
    }

    void _PCframe::getLattice(void *p)
    {
    }

    void _PCframe::paintPC(PointCloud *pPC)
    {
        NULL_(pPC);

        pPC->PaintUniformColor(m_vShadeCol.v3f().cast<double>());
    }

    int _PCframe::size(void)
    {
        return m_sPC.get()->points_.size();
    }

}
#endif
