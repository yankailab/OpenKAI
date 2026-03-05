/*
 * _PCframe.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_PCframe.h"
#include "_PCstream.h"
#include "_PCgridBase.h"

namespace kai
{

    _PCframe::_PCframe()
    {
        m_type = pc_frame;
        m_tStamp = 0;

        m_nPresv = 0;
        m_nPresvNext = 0;
    }

    _PCframe::~_PCframe()
    {
    }

    bool _PCframe::init(const json &j)
    {
        IF_F(!this->_GeometryBase::init(j));

        jKv(j, "nPresv", m_nPresv);
        jKv(j, "nPresvNext", m_nPresvNext);

        return true;
    }

    bool _PCframe::link(const json &j, ModuleMgr *pM)
    {
        IF_F(!this->_GeometryBase::link(j, pM));

        return initGeometry();
    }

    bool _PCframe::initGeometry(void)
    {
        mutexLock();

        // init
        if (m_nPresv > 0)
            m_sPC.get()->init(m_nPresv);

        if (m_nPresvNext > 0)
            m_sPC.next()->init(m_nPresvNext);

        m_sPC.get()->clear();
        m_sPC.next()->clear();

        mutexUnlock();

        return true;
    }

    void _PCframe::clear(void)
    {
        mutexLock();

        m_sPC.get()->clear();
        m_sPC.next()->clear();

        mutexUnlock();
    }

    bool _PCframe::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    bool _PCframe::check(void)
    {
        return this->_GeometryBase::check();
    }

    void _PCframe::update(void)
    {
        while (m_pT->bAlive())
        {
            m_pT->autoFPS();

            updatePCframe();
        }
    }

    void _PCframe::updatePCframe(void)
    {
        IF_(!check());

        readSharedMem();
        writeSharedMem();
    }

    void _PCframe::addPCstream(void *p, const uint64_t &tExpire)
    {
        NULL_(p);
        _PCstream *pS = (_PCstream *)p;

        mutexLock();

        POINT_CLOUD *pPC = m_sPC.next();
        pPC->clear();

        pS->copyTo(pPC, tExpire);

        mutexUnlock();

        swapBuffer();
    }

    void _PCframe::addPCframe(void *p)
    {
        NULL_(p);
        _PCframe *pF = (_PCframe *)p;

        mutexLock();
        pF->copyTo(m_sPC.next());
        mutexUnlock();

        swapBuffer();
    }

    void _PCframe::addPCgrid(void *p)
    {
    }

    void _PCframe::writeSharedMem(void)
    {
        NULL_(m_pSM);
        IF_(!m_pSM->bOpen());
        IF_(!m_pSM->bWriter());

        POINT_CLOUD *pPC = m_sPC.get();
        int nPw = pPC->m_vP.size();
        nPw = small<int>(nPw, m_nPresv);
        nPw = small<int>(nPw, m_pSM->nB() / sizeof(GEOMETRY_POINT));

        GEOMETRY_POINT* pGdst = (GEOMETRY_POINT*)m_pSM->p();
        for (int i = 0; i < nPw; i++)
        {
            pGdst[i] = pPC->m_vP[i];
        }

//        memcpy(m_pSM->p(), m_pGpSM, nPw * sizeof(GEOMETRY_POINT));
    }

    void _PCframe::readSharedMem(void)
    {
        NULL_(m_pSM);
        IF_(!m_pSM->bOpen());
        IF_(m_pSM->bWriter());

        GEOMETRY_POINT* pSrc = (GEOMETRY_POINT*)m_pSM->p();
        NULL_(pSrc);
//        memcpy(m_pGpSM, pSrc, m_nPresv * sizeof(GEOMETRY_POINT));

        mutexLock();

        POINT_CLOUD *pPC = m_sPC.next();
        pPC->m_vP.clear();

        for (int i = 0; i < m_nPresv; i++)
        {
            pPC->m_vP.push_back(pSrc[i]);
        }

        mutexUnlock();

        swapBuffer();
    }

    bool _PCframe::save2file(const string &fName)
    {
        IF_F(fName.empty());

        PointCloud pc;
        this->copyTo(&pc);

        io::WritePointCloudOption par;
        par.write_ascii = io::WritePointCloudOption::IsAscii::Binary;
        par.compressed = io::WritePointCloudOption::Compressed::Uncompressed;

        return io::WritePointCloudToPLY(fName.c_str(), pc, par);
    }

    void _PCframe::swapBuffer(void)
    {
        mutexLock();

        m_sPC.swap();
        m_sPC.next()->clear();

        mutexUnlock();
    }

    POINT_CLOUD *_PCframe::getBuffer(void)
    {
        return m_sPC.get();
    }

    POINT_CLOUD *_PCframe::getNextBuffer(void)
    {
        return m_sPC.next();
    }

    void _PCframe::copyTo(POINT_CLOUD *pDst)
    {
        IF_(!check());
        NULL_(pDst);

        *pDst = *m_sPC.get();
    }

    void _PCframe::copyTo(PointCloud *pDst)
    {
        IF_(!check());
        NULL_(pDst);

        pDst->Clear();
        pDst->points_.clear();
        pDst->colors_.clear();

        vector<GEOMETRY_POINT>* pvGP = &m_sPC.get()->m_vP;
        for(int i=0; i<pvGP->size(); i++)
        {
            GEOMETRY_POINT* pGp = &(*pvGP)[i];
			pDst->points_.push_back(v2e(pGp->m_vP).cast<double>());
			pDst->colors_.push_back(v2e(pGp->m_vC).cast<double>());
        }
    }

    int _PCframe::nP(void)
    {
        return m_sPC.get()->m_vP.size();
    }

    int _PCframe::nPnext(void)
    {
        return m_sPC.next()->m_vP.size();
    }

    int _PCframe::nPmax(void)
    {
        return m_nPresv;
    }
}
