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
        pthread_mutex_init(&m_mutexPC, NULL);
    }

    _PCframe::~_PCframe()
    {
        pthread_mutex_destroy(&m_mutexPC);
    }

    bool _PCframe::init(void *pKiss)
    {
        IF_F(!this->_GeometryBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        // frame buf
        pK->v("nPresv", &m_nPresv);
        if (m_nPresv > 0)
        {
            m_sPC.get()->points_.reserve(m_nPresv);
            m_sPC.get()->colors_.reserve(m_nPresv);

            m_nPresvNext = m_nPresv;
            pK->v("nPresvNext", &m_nPresvNext);
            if (m_nPresvNext > 0)
            {
                m_sPC.next()->points_.reserve(m_nPresvNext);
                m_sPC.next()->colors_.reserve(m_nPresvNext);
            }
        }

        return true;
    }

    bool _PCframe::link(void)
    {
        IF_F(!this->_GeometryBase::link());
        Kiss *pK = (Kiss *)m_pKiss;

        return true;
    }

    int _PCframe::check(void)
    {
        return this->_GeometryBase::check();
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
//        m_sPC.next()->Transform(m_mT);

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

        // _PCstream *pS = (_PCstream *)p;
        // PointCloud *pPC = m_sPC.next();

        // for (int i = 0; i < pS->m_nP; i++)
        // {
        //     GEOMETRY_POINT *pP = &pS->m_pP[i];

        //     pPC->points_.push_back(pP->m_vP);
        //     pPC->colors_.push_back(pP->m_vC.cast<double>());
        // }
    }

    void _PCframe::getFrame(void *p)
    {
        NULL_(p);

        _PCframe *pF = (_PCframe *)p;
        PointCloud pc;
        pF->getPC(&pc);

        *m_sPC.next() += pc;
    }

    void _PCframe::getGrid(void *p)
    {
    }

    int _PCframe::size(void)
    {
        return m_sPC.get()->points_.size();
    }
}
