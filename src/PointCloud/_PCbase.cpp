/*
 * _PCbase.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_PCbase.h"

#ifdef USE_OPEN3D

namespace kai
{

    _PCbase::_PCbase()
    {
        m_pPCB = NULL;
        m_iPr = 0;

        m_ring.init();

        m_bTransform = false;
        m_vT.init(0);
        m_vR.init(0);
        m_A = Eigen::Matrix4d::Identity();
    }

    _PCbase::~_PCbase()
    {
        m_ring.release();
    }

    bool _PCbase::init(void *pKiss)
    {
        IF_F(!this->_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;
        
        int nP = 0;
        pK->v("nP", &nP);
        IF_F(!m_ring.setup(nP));

        //transform
        pK->v("bTransform", &m_bTransform);
        pK->v("vT", &m_vT);
        pK->v("vR", &m_vR);

        string n;
        n = "";
        pK->v("_PCbase", &n);
        m_pPCB = (_PCbase *)(pK->getInst(n));

        return true;
    }

    int _PCbase::check(void)
    {
        return this->_ModuleBase::check();
    }

    void _PCbase::add(Eigen::Vector3d &vP, Eigen::Vector3d &vC, uint64_t &tStamp)
    {
        Vector3d p = m_A * vP;
        m_ring.add(p, vC, tStamp);
    }

    void _PCbase::setTranslation(vDouble3 &vT, vDouble3 &vR)
    {
        m_vT = vT;
        m_vR = vR;

        Eigen::Matrix4d mT;
        Eigen::Vector3d eR(m_vR.x, m_vR.y, m_vR.z);
        mT.block(0, 0, 3, 3) = Geometry3D::GetRotationMatrixFromXYZ(eR);
        mT(0, 3) = m_vT.x;
        mT(1, 3) = m_vT.y;
        mT(2, 3) = m_vT.z;

        m_A = mT;
    }

    PC_RING* _PCbase::getRing(void)
    {
        return &m_ring;
    }

    void _PCbase::readSrc(void)
    {
        NULL_(m_pPCB);
        m_ring.readSrc(m_pPCB->getRing(), &m_iPr);        
    }

    void _PCbase::draw(void)
    {
        this->_ModuleBase::draw();
    }

}
#endif
