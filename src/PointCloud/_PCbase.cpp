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
        m_vFrgb.init(1.0);
        m_vCrgb.init(0.0);
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

        //RGB
        pK->v("vToffsetRGB", &m_vToffsetRGB);
        pK->v("vRoffsetRGB", &m_vRoffsetRGB);
        setRGBoffset(m_vToffsetRGB, m_vRoffsetRGB);

        pK->v("vFrgb", &m_vFrgb);
        pK->v("vCrgb", &m_vCrgb);
        string yml;
        pK->v("yml", &yml);
        if (!yml.empty())
        {
            FileStorage fs(yml.c_str(), FileStorage::READ);
            if (fs.isOpened())
            {
                Mat mC;
                fs["mC"] >> mC;
                fs.release();
                m_vFrgb.x = mC.at<double>(0, 0);
                m_vFrgb.y = mC.at<double>(1, 1);
                m_vCrgb.x = mC.at<double>(0, 2);
                m_vCrgb.y = mC.at<double>(1, 2);
            }
        }

        //pipeline ctx
        pK->v("ctxdT", &m_pInCtx.m_dT);

        string n;
        n = "";
        pK->v("_PCbase", &n);
        m_pInCtx.m_pPCB = (_PCbase *)(pK->getInst(n));

        m_nPread = 0;

        //RGB map
        n = "";
        pK->v("_VisionBase", &n);
        m_pV = (_VisionBase *)(pK->getInst(n));

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

    bool _PCbase::getColor(const Vector3d &vP, Vector3f *pvC)
    {
        NULL_F(m_pV);
        NULL_F(m_pV->BGR());
        IF_F(m_pV->BGR()->bEmpty());
        IF_F(m_pV->getType() != vision_remap);

        Vector3d vPrgb = m_AoffsetRGB * vP;
        Vector3d vPa = Vector3d(
            vPrgb[m_vAxisIdxRGB.x] * m_vAxisKrgb.x,
            vPrgb[m_vAxisIdxRGB.y] * m_vAxisKrgb.y,
            vPrgb[m_vAxisIdxRGB.z] * m_vAxisKrgb.z);

        Mat *pM = m_pV->BGR()->m();
        float w = pM->cols;
        float h = pM->rows;

        float ovZ = 1.0 / vPa[2];
        float x = w * (m_vFrgb.x * vPa[0] + m_vCrgb.x * vPa[2]) * ovZ;
        float y = h * (m_vFrgb.y * vPa[1] + m_vCrgb.y * vPa[2]) * ovZ;
        x += float(pM->cols) * 0.5;
        y += float(pM->rows) * 0.5;

        IF_F(x < 0);
        IF_F(x > pM->cols - 1);
        IF_F(y < 0);
        IF_F(y > pM->rows - 1);

        Vec3b vC = pM->at<Vec3b>((int)y, (int)x);
        Vector3f vCf(vC[2], vC[1], vC[0]);
        vCf *= 1.0 / 255.0;
        *pvC = vCf;

        return true;
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
