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
        
        m_vColOvrr.init(-1.0);
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

        pK->v("vColOvrr", &m_vColOvrr);

        //frame buf
        int nPCreserve = 0;
        pK->v("nPCreserve", &nPCreserve);
        if (nPCreserve > 0)
        {
            m_sPC.prev()->points_.reserve(nPCreserve);
            m_sPC.prev()->colors_.reserve(nPCreserve);
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
        *pPC = *m_sPC.prev();
        pthread_mutex_unlock(&m_mutexPC);
    }

    void _PCframe::updatePC(void)
    {
        paintPC(m_sPC.next());

        pthread_mutex_lock(&m_mutexPC);
        m_sPC.update();
        m_sPC.next()->points_.clear();
        m_sPC.next()->colors_.clear();
        m_sPC.next()->normals_.clear();
        pthread_mutex_unlock(&m_mutexPC);
    }

    void _PCframe::getStream(void *p)
    {
        NULL_(p);

        _PCstream* pS = (_PCstream*)p;
        PointCloud* pPC = m_sPC.next();
        uint64_t tFrom = m_pT->getTfrom() - m_pInCtx.m_dT;
        if(m_pInCtx.m_dT >= UINT64_MAX)
            tFrom = 0;
            
        for (int i = 0; i < pS->m_nP; i++)
        {
            PC_POINT *pP = &pS->m_pP[i];
            IF_CONT(pP->m_tStamp < tFrom);

            pPC->points_.push_back(pP->m_vP);
            pPC->colors_.push_back(pP->m_vC);
        }
    }

    void _PCframe::getFrame(void *p)
    {
        NULL_(p);

        _PCframe* pF = (_PCframe*)p;
        PointCloud pc;
        pF->getPC(&pc);

        *m_sPC.next() += pc;
    }

    void _PCframe::getLattice(void *p)
    {
    }

    void _PCframe::paintPC(PointCloud *pPC)
    {
        NULL_(pPC);
        IF_(m_vColOvrr.x < 0.0)

        pPC->PaintUniformColor(
            Vector3d(
                m_vColOvrr.x,
                m_vColOvrr.y,
                m_vColOvrr.z));
    }

    int _PCframe::size(void)
    {
        return m_sPC.prev()->points_.size();
    }

    void _PCframe::draw(void)
    {
        this->_PCbase::draw();
    }

}
#endif
