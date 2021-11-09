/*
 * _MeshBase.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_MeshBase.h"

namespace kai
{

    _MeshBase::_MeshBase()
    {
        m_type = mesh_unknown;

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
    }

    _MeshBase::~_MeshBase()
    {
    }

    bool _MeshBase::init(void *pKiss)
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

        //transform
        pK->v("vT", &m_vT);
        pK->v("vR", &m_vR);
        setTranslation(m_vT, m_vR);

        //pipeline ctx
        pK->v("ctxdT", &m_pInCtx.m_dT);

        string n;
        n = "";
        pK->v("_MeshBase", &n);
        m_pInCtx.m_pPCB = (_MeshBase *)(pK->getInst(n));

        m_nPread = 0;

        //RGB
        n = "";
        pK->v("_VisionBase", &n);
        m_pV = (_VisionBase *)(pK->getInst(n));

        n = "";
        pK->v("fCalib", &n);
        Kiss *pKf = new Kiss();
        if (parseKiss(n, pKf))
        {
            pK = pKf->child("calib");
            IF_d_T(pK->empty(), DEL(pKf));

            pK->v("vOffsetPt", &m_vToffset);
            pK->v("vOffsetPr", &m_vRoffset);
            pK->v("vOffsetCt", &m_vToffsetRGB);
            pK->v("vOffsetCr", &m_vRoffsetRGB);
        }
        DEL(pKf);

        setOffset(m_vToffset, m_vRoffset);
        setRGBoffset(m_vToffsetRGB, m_vRoffsetRGB);

        return true;
    }

    int _MeshBase::check(void)
    {
        return this->_ModuleBase::check();
    }

    MESH_TYPE _MeshBase::getType(void)
    {
        return m_type;
    }

    Matrix4d _MeshBase::getTranslationMatrix(const vDouble3 &vT, const vDouble3 &vR)
    {
        Matrix4d mT = Matrix4d::Identity();
        Vector3d eR(vR.x, vR.y, vR.z);
        mT.block(0, 0, 3, 3) = Geometry3D::GetRotationMatrixFromXYZ(eR);
        mT(0, 3) = vT.x;
        mT(1, 3) = vT.y;
        mT(2, 3) = vT.z;

        return mT;
    }

    void _MeshBase::setOffset(const vDouble3 &vT, const vDouble3 &vR)
    {
        m_vToffset = vT;
        m_vRoffset = vR;
        m_mToffset = getTranslationMatrix(vT, vR);
        m_Aoffset = m_mToffset;
    }

    void _MeshBase::setTranslation(const vDouble3 &vT, const vDouble3 &vR)
    {
        m_vT = vT;
        m_vR = vR;
        m_mT = getTranslationMatrix(m_vT, m_vR) * m_mToffset;
        m_A = m_mT;
    }

    void _MeshBase::setTranslation(const Matrix4d &mT)
    {
        m_mT = mT * m_mToffset;
        m_A = m_mT;
    }

    void _MeshBase::setRGBoffset(const vDouble3 &vT, const vDouble3 &vR)
    {
        m_vToffsetRGB = vT;
        m_vRoffsetRGB = vR;
        m_mToffsetRGB = getTranslationMatrix(vT, vR);
        m_AoffsetRGB = m_mToffsetRGB;
    }

    void _MeshBase::clear(void)
    {
    }

    void _MeshBase::getStream(void *p)
    {
    }

    bool _MeshBase::getColor(const Vector3d &vP, Vector3f *pvC)
    {
        NULL_F(m_pV);
        NULL_F(m_pV->BGR());
        IF_F(m_pV->BGR()->bEmpty());
        IF_F(m_pV->getType() != vision_remap);

        Vector3d vPrgb = m_AoffsetRGB * vP; //vP raw lidar coordinate
        Vector3d vPa = Vector3d(            //transform to RGB coordinate
            vPrgb[m_vAxisIdxRGB.x] * m_vAxisKrgb.x,
            vPrgb[m_vAxisIdxRGB.y] * m_vAxisKrgb.y,
            vPrgb[m_vAxisIdxRGB.z] * m_vAxisKrgb.z);

        Mat *pM = m_pV->BGR()->m();
        float w = pM->cols;
        float h = pM->rows;
        vDouble2 vFrgb = m_pV->getF();
        vDouble2 vCrgb = m_pV->getC();

        float ovZ = 1.0 / vPa[2];
        float x = vFrgb.x * vPa[0] * ovZ + vCrgb.x;
        float y = vFrgb.y * vPa[1] * ovZ + vCrgb.y;

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

    bool _MeshBase::bRange(const Vector3d &vP)
    {
        double ds = vP[0] * vP[0] + vP[1] * vP[1] + vP[2] * vP[2];
        IF_F(ds < m_vRange.x);
        IF_F(ds > m_vRange.y);

        return true;
    }

}
