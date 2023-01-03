/*
 * _PCstream.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

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

    int _PCstream::check(void)
    {
        return this->_GeometryBase::check();
    }

    void _PCstream::AcceptAdd(bool b)
    {
        m_bAccept = b;
    }

    void _PCstream::add(const Vector3d &vP, const Vector3f &vC, uint64_t tStamp)
    {
        NULL_(m_pP);
        IF_(!m_bAccept);

        // lidar to Nav coordinate
        Vector3d vPnav = Vector3d(
                            vP[m_vAxisIdx.x] * m_vAxisK.x,
                            vP[m_vAxisIdx.y] * m_vAxisK.y,
                            vP[m_vAxisIdx.z] * m_vAxisK.z
                            );
        IF_(!bRange(vPnav));

        Vector3f vCrgb = vC;
        // if(m_pR)
        // {
        //     IF_(!getColor(vP, &vCrgb));
        // }

        PC_POINT *pP = &m_pP[m_iP];
        pP->m_vP = m_A * vPnav;   // m_A incorporates the offset in Nav coordinate
        pP->m_vC = vCrgb;
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

    void _PCstream::getNextFrame(void* p)
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

    void _PCstream::refreshCol(void)
    {
        NULL_(m_pP);
//        NULL_(m_pR);

        vDouble3 vTr, vRr;
        vTr.set(-m_vToffset.x, -m_vToffset.y, -m_vToffset.z);
        vRr.set(-m_vRoffset.x, -m_vRoffset.y, -m_vRoffset.z);
        Eigen::Affine3d aRev;
        aRev = getTranslationMatrix(vTr, vRr);

        for (int i = 0; i < m_nP; i++)
        {
            PC_POINT *pP = &m_pP[i];
            IF_CONT(pP->m_tStamp <= 0);

            Vector3d vPr = aRev * pP->m_vP;
            Vector3d vP;
            vP[m_vAxisIdx.x] = vPr[0] / m_vAxisK.x;
            vP[m_vAxisIdx.y] = vPr[1] / m_vAxisK.y;
            vP[m_vAxisIdx.z] = vPr[2] / m_vAxisK.z;            

            Vector3f vC;
            if(getColor(vP, &vC))
                pP->m_vC = vC;
            else
                pP->m_vC = Vector3f(0,0,0);
        }
    }

    int _PCstream::nP(void)
    {
        return m_nP;
    }

    int _PCstream::iP(void)
    {
        return m_iP;
    }

    void _PCstream::startStream(void)
    {
    }

    void _PCstream::stopStream(void)
    {
    }

}
