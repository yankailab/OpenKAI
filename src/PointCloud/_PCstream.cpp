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
        m_nP = 0;
        m_iP = 0;
        m_tLastUpdate = 0;
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

        int nP = 0;
        pK->v("nP", &nP);
        IF_F(nP <= 0);

        m_nP = nP;
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

    void _PCstream::add(Vector3d &vP, Vector3d &vC, uint64_t tStamp)
    {
        NULL_(m_pP);

        PC_POINT *pP = &m_pP[m_iP];
        pP->m_vP = (m_bTransform) ? (m_A * vP) : vP;
        pP->m_vC = vC;
        pP->m_tStamp = tStamp;

        m_iP = iInc(m_iP, m_nP);
    }

    void _PCstream::getStream(void* p)
    {
        NULL_(p);

        _PCstream* pS = (_PCstream*)p;
        PC_POINT* pP = pS->m_pP;
        while (m_pInCtx.m_iPr != pS->m_iP)
        {
            PC_POINT po = pP[m_pInCtx.m_iPr];

            if (po.m_tStamp >= m_pInCtx.m_tFrom)
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

    void _PCstream::draw(void)
    {
        this->_ModuleBase::draw();
    }

}
#endif
