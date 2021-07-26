/*
 * _ARarea.cpp
 *
 *  Created on: July 26, 2021
 *      Author: yankai
 */

#include "_ARarea.h"

#ifdef USE_OPENCV

namespace kai
{

	_ARarea::_ARarea()
	{
		m_pV = NULL;
		m_pD = NULL;
		m_pS = NULL;

        m_vRange.init(0.0, 1000.0);
		m_vEyeOrigin = {0,0,0};
		m_vEye = {0,0,0};
		m_vPtAt = {0,0,0};

        m_vToffsetRGB.init(0);
        m_vRoffsetRGB.init(0);
        m_mToffsetRGB = Matrix4d::Identity();
        m_vAxisIdxRGB.init(0, 1, 2);
        m_vAxisKrgb.init(1.0);

	}

	_ARarea::~_ARarea()
	{
	}

	bool _ARarea::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

        pK->v("vRange", &m_vRange);
        pK->v("vAxisIdxRGB", &m_vAxisIdxRGB);
        pK->v("vAxisKrgb", &m_vAxisKrgb);

		string n;

        pK->v("fCalib", &n);
		_File *pF = new _File();
		IF_d_T(!pF->open(&n, ios::in), DEL(pF));
		IF_d_T(!pF->readAll(&n), DEL(pF));
		IF_d_T(n.empty(), DEL(pF));
		pF->close();
		DEL(pF);

		Kiss *pKf = new Kiss();
		IF_d_T(!pKf->parse(&n),DEL(pKf));

		pK = pKf->child("calib");
		IF_d_T(pK->empty(), DEL(pKf));

        pK->v("vOffsetCt", &m_vToffsetRGB);
        pK->v("vOffsetCr", &m_vRoffsetRGB);
		DEL(pKf);

        m_mToffsetRGB = Matrix4d::Identity();
//        Vector3d eR(m_vRoffsetRGB.x, m_vRoffsetRGB.y, m_vRoffsetRGB.z);
//        m_mToffsetRGB.block(0, 0, 3, 3) = Geometry3D::GetRotationMatrixFromXYZ(eR);
        m_mToffsetRGB(0, 3) = m_vToffsetRGB.x;
        m_mToffsetRGB(1, 3) = m_vToffsetRGB.y;
        m_mToffsetRGB(2, 3) = m_vToffsetRGB.z;
        m_AoffsetRGB = m_mToffsetRGB;


		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->getInst(n));
		IF_Fl(!m_pV, n + " not found");

		n = "";
		pK->v("_DistSensorBase", &n);
		m_pD = (_DistSensorBase *)(pK->getInst(n));
		IF_Fl(!m_pD, n + " not found");

		n = "";
		pK->v("_SlamBase", &n);
		m_pS = (_SlamBase *)(pK->getInst(n));
		IF_Fl(!m_pS, n + " not found");

		return true;
	}

	bool _ARarea::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _ARarea::check(void)
	{
		NULL__(m_pV, -1);
		IF__(m_pV->BGR()->bEmpty(), -1);
		NULL__(m_pD, -1);
		NULL__(m_pS, -1);

		return 0;
	}

	void _ARarea::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			updateARarea();

			m_pT->autoFPSto();
		}
	}

	bool _ARarea::updateARarea(void)
	{
		IF_F(check() < 0);

		m_d = m_pD->d((int)0);
		Vector3d vPtAt = m_vEyeOrigin;
		vPtAt[0] = m_d;

		Eigen::Affine3d A;
		A = m_pS->mT();
		m_vEye = A * m_vEyeOrigin;
		m_vPtAt = A * vPtAt;

		vInt2 vPtScr;
		IF_F(!world2Scr(m_vPtAt, &vPtScr));
		

		return false;
	}

    bool _ARarea::world2Scr(const Vector3d &vP, vInt2* vPscr)
    {
		IF_F(check() < 0);
		NULL_F(vPscr);

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
        float x = w * (vFrgb.x * vPa[0] + vCrgb.x * vPa[2]) * ovZ;
        float y = h * (vFrgb.y * vPa[1] + vCrgb.y * vPa[2]) * ovZ;

        IF_F(x < 0);
        IF_F(x > pM->cols - 1);
        IF_F(y < 0);
        IF_F(y > pM->rows - 1);

        Vec3b vC = pM->at<Vec3b>((int)y, (int)x);
    }

	void _ARarea::cvDraw(void *pWindow)
	{
		NULL_(pWindow);
		this->_ModuleBase::cvDraw(pWindow);
		IF_(check() < 0);

		_WindowCV *pWin = (_WindowCV *)pWindow;
		Frame *pF = pWin->getFrame();
		NULL_(pF);
		Mat *pM = pF->m();
		IF_(pM->empty());

		Scalar col = Scalar(0, 255, 0);

//		rectangle(*pM, r, col, 3);
	}

}
#endif
