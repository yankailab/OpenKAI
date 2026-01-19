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
        m_pGpSM = nullptr;

        m_nPresv = 0;
        m_nPresvNext = 0;
    }

    _PCframe::~_PCframe()
    {
    }

    bool _PCframe::init(const json &j)
    {
        IF_F(!this->_GeometryBase::init(j));

        jVar(j, "nPresv", m_nPresv);
        jVar(j, "nPresvNext", m_nPresvNext);

        return initGeometry();
    }

    bool _PCframe::initGeometry(void)
    {
        mutexLock();

        // frame buf reservation
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

        // init
        m_sPC.get()->points_.clear();
        m_sPC.get()->colors_.clear();
        m_sPC.get()->normals_.clear();

        m_sPC.next()->points_.clear();
        m_sPC.next()->colors_.clear();
        m_sPC.next()->normals_.clear();

        // share mem
        m_pGpSM = new GEOMETRY_POINT[m_nPresv];
        if (!m_pGpSM)
        {
            mutexUnlock();
            return false;
        }

        for (int i = 0; i < m_nPresv; i++)
            m_pGpSM[i].clear();

        mutexUnlock();

        return true;
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

        PointCloud *pPC = m_sPC.next();
        pPC->Clear();
        pPC->points_.clear();
        pPC->colors_.clear();

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

        PointCloud *pPC = m_sPC.get();
        int nPw = pPC->points_.size();
        nPw = small<int>(nPw, m_nPresv);
        nPw = small<int>(nPw, m_pSM->nB() / sizeof(GEOMETRY_POINT));

        uint64_t tNow = getTbootUs();

        for (int i = 0; i < nPw; i++)
        {
            GEOMETRY_POINT *pGP = &m_pGpSM[i];
            pGP->m_vP = e2v((Vector3f)pPC->points_[i].cast<float>());
            pGP->m_vC = e2v((Vector3f)pPC->colors_[i].cast<float>());
            pGP->m_tStamp = tNow;
        }

        memcpy(m_pSM->p(), m_pGpSM, nPw * sizeof(GEOMETRY_POINT));
    }

    void _PCframe::readSharedMem(void)
    {
        NULL_(m_pSM);
        IF_(!m_pSM->bOpen());
        IF_(m_pSM->bWriter());

        void *pSrc = m_pSM->p();
        NULL_(pSrc);

        memcpy(m_pGpSM, pSrc, m_nPresv * sizeof(GEOMETRY_POINT));

        mutexLock();

        PointCloud *pPC = m_sPC.next();
        pPC->Clear();
        pPC->points_.clear();
        pPC->colors_.clear();

        for (int i = 0; i < m_nPresv; i++)
        {
            GEOMETRY_POINT *pGP = &m_pGpSM[i];
            pPC->points_.push_back(v2e(pGP->m_vP).cast<double>());
            pPC->colors_.push_back(v2e(pGP->m_vC).cast<double>());
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
        m_sPC.next()->points_.clear();
        m_sPC.next()->colors_.clear();
        m_sPC.next()->normals_.clear();

        mutexUnlock();
    }

    PointCloud *_PCframe::getBuffer(void)
    {
        return m_sPC.get();
    }

    PointCloud *_PCframe::getNextBuffer(void)
    {
        return m_sPC.next();
    }

    void _PCframe::copyTo(PointCloud *pPC)
    {
        IF_(!check());
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

    int _PCframe::nPnext(void)
    {
        return m_sPC.next()->points_.size();
    }
}
