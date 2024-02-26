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
		pK->m_pInst = this;

        pK->v("bAccept", &m_bAccept);
        pK->v("nP", &m_nP);
        IF_F(m_nP <= 0);

        m_pP = new GEOMETRY_POINT[m_nP];
        NULL_F(m_pP);
        m_iP = 0;
        m_tLastUpdate = 0;

        for (int i = 0; i < m_nP; i++)
            m_pP[i].clear();

        return true;
    }

    int _MeshStream::check(void)
    {
        return this->_GeometryBase::check();
    }

    void _MeshStream::setAccept(bool b)
    {
        m_bAccept = b;
    }

    int _MeshStream::addVertex(const Vector3d &vP, uint64_t tStamp)
    {
        NULL__(m_pP, -1);
        IF__(!m_bAccept, -1);

        return 0;
    }

    int _MeshStream::addTriangle(const vInt3& vVertices, uint64_t tStamp)
    {
        return -1;
    }

    void _MeshStream::getStream(void* p)
    {
        NULL_(p);

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
            m_pP[i].clear();

        m_iP = 0;
    }

}
