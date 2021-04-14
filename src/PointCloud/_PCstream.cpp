/*
 * _PCstream.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifdef USE_OPEN3D
#include "_PCstream.h"

namespace kai
{

    _PCstream::_PCstream()
    {
        m_type = pc_stream;

        m_pP = NULL;
        m_nP = 256;
        m_iP = 0;
        m_tLastUpdate = 0;
        m_bAccept = true;
    }

    _PCstream::~_PCstream()
    {
        m_nP = 0;
        DEL(m_pP);
    }

    bool _PCstream::init(void *pKiss)
    {
        IF_F(!this->_PCbase::init(pKiss));
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

    int _PCstream::check(void)
    {
        return this->_PCbase::check();
    }

    void _PCstream::AcceptAdd(bool b)
    {
        m_bAccept = b;
    }

    void _PCstream::add(const Vector3d &vP, const Vector3d &vC, uint64_t tStamp)
    {
        NULL_(m_pP);
        IF_(!m_bAccept);

        Vector3d uP = Vector3d(
                            vP[m_vAxisIdx.x] * m_vAxisK.x,
                            vP[m_vAxisIdx.y] * m_vAxisK.y,
                            vP[m_vAxisIdx.z] * m_vAxisK.z
                            );
        IF_(!bRange(uP));

        PC_POINT *pP = &m_pP[m_iP];
        pP->m_vP = m_A * uP;
        pP->m_vC = vC;
        pP->m_tStamp = tStamp;

        m_iP = iInc(m_iP, m_nP);
        m_nPread++;
    }

    void _PCstream::getStream(void* p)
    {
        NULL_(p);

        _PCstream* pS = (_PCstream*)p;
        PC_POINT* pP = pS->m_pP;
        uint64_t tFrom = m_pT->getTfrom() - m_pInCtx.m_dT;
        while (m_pInCtx.m_iPr != pS->m_iP)
        {
            PC_POINT po = pP[m_pInCtx.m_iPr];

            if (po.m_tStamp >= tFrom)
            {
                m_pP[m_iP] = po;
                m_iP = iInc(m_iP, m_nP);
            }

            m_pInCtx.m_iPr = iInc(m_pInCtx.m_iPr, pS->m_nP);
        }
    }

    void _PCstream::getFrame(void* p)
    {
    }

    void _PCstream::getLattice(void* p)
    {
    }

    void _PCstream::clear(void)
    {
        for(int i=0; i<m_nP; i++)
            m_pP[i].init();

        m_iP = 0;
    }

    int _PCstream::nP(void)
    {
        return m_nP;
    }

    int _PCstream::iP(void)
    {
        return m_iP;
    }

    void _PCstream::draw(void)
    {
        this->_ModuleBase::draw();
    }

}
#endif
