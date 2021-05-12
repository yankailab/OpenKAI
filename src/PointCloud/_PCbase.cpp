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
        m_mToffset = Matrix4d::Identity();
        m_Aoffset = Matrix4d::Identity();

        m_vT.init(0);
        m_vR.init(0);
        m_mT = Matrix4d::Identity();
        m_A = Matrix4d::Identity();

        m_pV = NULL;
        m_vToffsetRGB.init(0);
        m_vRoffsetRGB.init(0);
        m_mToffsetRGB = Matrix4d::Identity();

        m_vAxisIdx.init(0, 1, 2);
        m_vAxisK.init(1.0);
        m_unitK = 1.0;
        m_vRange.init(0.0, 1000.0);

        m_vAxisIdxRGB.init(0, 1, 2);
        m_vAxisKrgb.init(1.0);

        m_pInCtx.init();
        m_pV = NULL;
    }

    _PCbase::~_PCbase()
    {
    }

    bool _PCbase::init(void *pKiss)
    {
        IF_F(!this->_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("unitK", &m_unitK);
        pK->v("vAxisIdx", &m_vAxisIdx);
        pK->v("vAxisK", &m_vAxisK);
        m_vAxisK *= m_unitK;
        pK->v("vAxisIdxRGB", &m_vAxisIdxRGB);
        pK->v("vAxisKrgb", &m_vAxisKrgb);
        m_vAxisKrgb *= m_unitK;

        pK->v("vRange", &m_vRange);
        m_vRange.x *= m_vRange.x;
        m_vRange.y *= m_vRange.y;

        //origin offset
        pK->v("vToffset", &m_vToffset);
        pK->v("vRoffset", &m_vRoffset);
        setOffset(m_vToffset, m_vRoffset);

        //transform
        pK->v("vT", &m_vT);
        pK->v("vR", &m_vR);
        setTranslation(m_vT, m_vR);

        //RGB offset
        pK->v("vToffsetRGB", &m_vToffsetRGB);
        pK->v("vRoffsetRGB", &m_vRoffsetRGB);
        setRGBoffset(m_vToffsetRGB, m_vRoffsetRGB);

        //pipeline ctx
        pK->v("ctxdT", &m_pInCtx.m_dT);

        string n;
        n = "";
        pK->v("_PCbase", &n);
        m_pInCtx.m_pPCB = (_PCbase *)(pK->getInst(n));

        m_nPread = 0;

        //RGB map
        n = "";
        pK->v("_Remap", &n);
        m_pV = (_Remap *)(pK->getInst(n));

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

    Matrix4d _PCbase::getTranslationMatrix(const vDouble3 &vT, const vDouble3 &vR)
    {
        Matrix4d mT = Matrix4d::Identity();
        Vector3d eR(vR.x, vR.y, vR.z);
        mT.block(0, 0, 3, 3) = Geometry3D::GetRotationMatrixFromXYZ(eR);
        mT(0, 3) = vT.x;
        mT(1, 3) = vT.y;
        mT(2, 3) = vT.z;

        return mT;
    }

    void _PCbase::setOffset(const vDouble3 &vT, const vDouble3 &vR)
    {
        m_vToffset = vT;
        m_vRoffset = vR;
        m_mToffset = getTranslationMatrix(vT, vR);
        m_Aoffset = m_mToffset;
    }

    void _PCbase::setRGBoffset(const vDouble3 &vT, const vDouble3 &vR)
    {
        m_vToffsetRGB = vT;
        m_vRoffsetRGB = vR;
        m_mToffsetRGB = getTranslationMatrix(vT, vR);
        m_AoffsetRGB = m_mToffsetRGB;
    }

    void _PCbase::setTranslation(const vDouble3 &vT, const vDouble3 &vR)
    {
        m_vT = vT;
        m_vR = vR;
        m_mT = getTranslationMatrix(m_vT, m_vR) * m_mToffset;
        m_A = m_mT;
    }

    void _PCbase::setTranslation(const Matrix4d &mT)
    {
        m_mT = mT * m_mToffset;
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

    int _PCbase::nPread(void)
    {
        return m_nPread;
    }

    void _PCbase::clear(void)
    {
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

    Vector3f _PCbase::getColor(const Vector3d &vP)
    {
        Vector3f vN(1, 1, 1);
        NULL__(m_pV, vN);
        NULL__(m_pV->BGR(), vN);
        IF__(m_pV->BGR()->bEmpty(), vN);
        IF__(m_pV->getType() != vision_remap, vN);

        Vector3d vPrgb = m_AoffsetRGB * vP;
        Vector3d vPa = Vector3d(
            vPrgb[m_vAxisIdxRGB.x] * m_vAxisKrgb.x,
            vPrgb[m_vAxisIdxRGB.y] * m_vAxisKrgb.y,
            vPrgb[m_vAxisIdxRGB.z] * m_vAxisKrgb.z);

        Mat mC = m_pV->mC();
        double Fx = mC.at<double>(0, 0);
        double Fy = mC.at<double>(1, 1);
        double Cx = mC.at<double>(0, 2);
        double Cy = mC.at<double>(1, 2);

        float ovZ = 1.0/vPa[2];
        float x = (Fx*vPa[0] + Cx*vPa[2]) * ovZ;
        float y = (Fy*vPa[1] + Cy*vPa[2]) * ovZ;

        Mat* pM = m_pV->BGR()->m();
        x = constrain<float>(x + pM->cols/2, 0, pM->cols-1);
        y = constrain<float>(y + pM->rows/2, 0, pM->rows-1);

        Vec3b vC = pM->at<Vec3b>((int)y, (int)x);
        Vector3f vCf(vC[2], vC[1], vC[0]);
        vCf *= 1.0/255.0;

        return vCf;
    }

    bool _PCbase::bRange(const Vector3d &vP)
    {
        double ds = vP[0] * vP[0] + vP[1] * vP[1] + vP[2] * vP[2];
        IF_F(ds < m_vRange.x);
        IF_F(ds > m_vRange.y);

        return true;
    }

}
#endif
