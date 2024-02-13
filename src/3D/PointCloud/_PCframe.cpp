/*
 * _PCframe.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_PCframe.h"
#include "_PCstream.h"
#include "_PCgrid.h"

namespace kai
{

    _PCframe::_PCframe()
    {
        m_type = pc_frame;
        m_nPresv = 0;
        m_nPresvNext = 0;
        m_tStamp = NULL;
    }

    _PCframe::~_PCframe()
    {
    }

    bool _PCframe::init(void *pKiss)
    {
        IF_F(!this->_GeometryBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("nPresv", &m_nPresv);
        m_nPresvNext = m_nPresv;
        pK->v("nPresvNext", &m_nPresvNext);

        return true;
    }

    int _PCframe::check(void)
    {
        return this->_GeometryBase::check();
    }

    bool _PCframe::initBuffer(void)
    {
        mutexLock();

        if (m_nPresv > 0)
        {
            m_sPC.get()->points_.reserve(m_nPresv);
            m_sPC.get()->colors_.reserve(m_nPresv);
        }
        if (m_nPresvNext > 0)
        {
            m_sPC.next()->points_.reserve(m_nPresvNext);
            m_sPC.next()->colors_.reserve(m_nPresvNext);
        }

        mutexUnlock();

        return true;
    }

    void _PCframe::swapBuffer(void)
    {
        mutexLock();

        m_sPC.swap();
        m_sPC.next()->points_.clear();
        m_sPC.next()->colors_.clear();
        m_sPC.next()->normals_.clear();

        mutexUnlock();
    }

    void _PCframe::clear(void)
    {
        mutexLock();

        m_sPC.get()->points_.clear();
        m_sPC.get()->colors_.clear();
        m_sPC.get()->normals_.clear();

        m_sPC.next()->points_.clear();
        m_sPC.next()->colors_.clear();
        m_sPC.next()->normals_.clear();

        mutexUnlock();
    }

    void _PCframe::getStream(void *p, const uint64_t& tExpire)
    {
        IF_(check() < 0);
        NULL_(p);
        _PCstream *pS = (_PCstream *)p;

        mutexLock();

        PointCloud *pPC = m_sPC.next();
        uint64_t tNow = getApproxTbootUs();

        for (int i = 0; i < pS->nP(); i++)
        {
            GEOMETRY_POINT* pP = pS->get(i);
            IF_CONT(bExpired(pP->m_tStamp, tExpire, tNow));

            pPC->points_.push_back(pP->m_vP);
            pPC->colors_.push_back(pP->m_vC.cast<double>());
        }

        mutexUnlock();
    }

    void _PCframe::getFrame(void *p)
    {
        NULL_(p);

        _PCframe *pF = (_PCframe *)p;
        PointCloud pc;
        pF->copyTo(&pc);

        *m_sPC.next() += pc;
    }

    void _PCframe::getGrid(void *p)
    {
    }

    void _PCframe::copyTo(PointCloud *pPC)
    {
        IF_(check() < 0);
        NULL_(pPC);

        pPC->Clear();
        pPC->points_.clear();
        pPC->colors_.clear();

        *pPC = *m_sPC.get();
    }

    int _PCframe::nP(void)
    {
        return m_sPC.get()->points_.size();
    }
}
