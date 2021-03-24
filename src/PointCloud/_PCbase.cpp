/*
 * _PCbase.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifdef USE_OPEN3D
#include "_PCbase.h"

namespace kai
{

    _PCbase::_PCbase()
    {
        m_type = pc_unknown;
        m_vToffset.init(0);
        m_vRoffset.init(0);
        m_bTransform = false;
        m_vT.init(0);
        m_vR.init(0);
        m_mT = Matrix4d::Identity();
        m_A = Matrix4d::Identity();

        m_pInCtx.init();
    }

    _PCbase::~_PCbase()
    {
    }

    bool _PCbase::init(void *pKiss)
    {
        IF_F(!this->_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        //offset
        pK->v("vToffset", &m_vToffset);
        pK->v("vRoffset", &m_vRoffset);

        //transform
        pK->v("bTransform", &m_bTransform);
        pK->v("vT", &m_vT);
        pK->v("vR", &m_vR);
        if (m_bTransform)
            setTranslation(m_vT, m_vR);

        //pipeline ctx
        pK->v("ctxdT", &m_pInCtx.m_dT);

        string n;
        n = "";
        pK->v("_PCbase", &n);
        m_pInCtx.m_pPCB = (_PCbase *)(pK->getInst(n));

        return true;
    }

    int _PCbase::check(void)
    {
        return this->_ModuleBase::check();
    }

    PC_TYPE _PCbase::getType(void)
    {
        return m_type;
    }

    void _PCbase::setTranslation(vDouble3 &vT, vDouble3 &vR)
    {
        m_vT = vT;
        m_vR = vR;

        Matrix4d mT = Matrix4d::Identity();
        Vector3d eR(m_vR.x + m_vRoffset.x,
                    m_vR.y + m_vRoffset.y,
                    m_vR.z + m_vRoffset.z);
        mT.block(0, 0, 3, 3) = Geometry3D::GetRotationMatrixFromXYZ(eR);
        mT(0, 3) = m_vT.x + m_vToffset.x;
        mT(1, 3) = m_vT.y + m_vToffset.y;
        mT(2, 3) = m_vT.z + m_vToffset.z;

        m_mT = mT;
        m_A = m_mT;
    }

    void _PCbase::readPC(void *pPC)
    {
        NULL_(pPC);

        PC_TYPE t = ((_PCbase *)pPC)->getType();

        if (t == pc_stream)
            getStream(pPC);
        else if (t == pc_frame)
            getFrame(pPC);
        else if (t == pc_lattice)
            getLattice(pPC);
    }

    void _PCbase::getStream(void *p)
    {
    }

    void _PCbase::getFrame(void *p)
    {
    }

    void _PCbase::getLattice(void *p)
    {
    }

    void _PCbase::draw(void)
    {
        this->_ModuleBase::draw();
    }

}
#endif
