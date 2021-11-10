/*
 * _MeshStream.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_MeshStream.h"

namespace kai
{
    _MeshStream::_MeshStream()
    {
        m_type = pc_stream;

        m_pP = NULL;
        m_nP = 256;
        m_iP = 0;
        m_tLastUpdate = 0;
        m_bAccept = true;
    }

    _MeshStream::~_MeshStream()
    {
        m_nP = 0;
        DEL(m_pP);
    }

    bool _MeshStream::init(void *pKiss)
    {
        IF_F(!this->_GeometryBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("bAccept", &m_bAccept);
        pK->v("nP", &m_nP);
        IF_F(m_nP <= 0);

        m_pP = new PC_POINT[m_nP];
        NULL_F(m_pP);
        m_iP = 0;
        m_tLastUpdate = 0;

        for (int i = 0; i < m_nP; i++)
            m_pP[i].init();

        return true;
    }

    int _MeshStream::check(void)
    {
        return this->_GeometryBase::check();
    }

    void _MeshStream::AcceptAdd(bool b)
    {
        m_bAccept = b;
    }

    int _MeshStream::addVertex(const Vector3d &vP, uint64_t tStamp)
    {
        NULL__(m_pP, -1);
        IF__(!m_bAccept, -1);

        Vector3d vPa = Vector3d(
                            vP[m_vAxisIdx.x] * m_vAxisK.x,
                            vP[m_vAxisIdx.y] * m_vAxisK.y,
                            vP[m_vAxisIdx.z] * m_vAxisK.z
                            );
        IF__(!bRange(vPa), -1);

        PC_POINT *pP = &m_pP[m_iP];
        pP->m_vP = m_A * vPa;
        pP->m_tStamp = tStamp;

//        m_iP = iInc(m_iP, m_nP);
//        m_nPread++;
        return 0;
    }

    int _MeshStream::addTriangle(const vInt3& vVertices, uint64_t tStamp)
    {
        return -1;
    }

    void _MeshStream::getStream(void* p)
    {
        NULL_(p);

        _MeshStream* pS = (_MeshStream*)p;
        PC_POINT* pP = pS->m_pP;
        uint64_t tFrom = m_pT->getTfrom() - m_pInCtx.m_dT;
        while (m_pInCtx.m_iPr != pS->m_iP)
        {
            PC_POINT po = pP[m_pInCtx.m_iPr];

            if (po.m_tStamp >= tFrom)
            {
                m_pP[m_iP] = po;
//                m_iP = iInc(m_iP, m_nP);
            }

//            m_pInCtx.m_iPr = iInc(m_pInCtx.m_iPr, pS->m_nP);
        }
    }

    void _MeshStream::getNextFrame(void* p)
    {
    }

    void _MeshStream::getLattice(void* p)
    {
    }

    int _MeshStream::nP(void)
    {
        return m_nP;
    }

    int _MeshStream::iP(void)
    {
        return m_iP;
    }

    void _MeshStream::startStream(void)
    {
    }

    void _MeshStream::stopStream(void)
    {
    }

    void _MeshStream::clear(void)
    {
        for(int i=0; i<m_nP; i++)
            m_pP[i].init();

        m_iP = 0;
    }

}
