/*
 * _VisionBase.cpp
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#ifdef USE_OPENCV
#include "_VisionBase.h"

namespace kai
{

	_VisionBase::_VisionBase()
	{
		m_bOpen = false;
		m_type = vision_unknown;
		m_vSize.init(1280, 720);
		m_bbDraw.x = -1.0;
	}

	_VisionBase::~_VisionBase()
	{
	}

	bool _VisionBase::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vSize", &m_vSize);
		pK->v("bbDraw", &m_bbDraw);

		m_bOpen = false;

		string n;
		pK->v("fCalib", &n);
		Kiss *pKf = new Kiss();
		if (parseKiss(n, pKf))
		{
			pK = pKf->child("calib");
			IF_d_T(pK->empty(), DEL(pKf));

			Mat mC = Mat::zeros(3, 3, CV_64FC1);
			pK->v("Fx", &mC.at<double>(0, 0));
			pK->v("Fy", &mC.at<double>(1, 1));
			pK->v("Cx", &mC.at<double>(0, 2));
			pK->v("Cy", &mC.at<double>(1, 2));
			mC.at<double>(2, 2) = 1.0;

			Mat mD = Mat::zeros(1, 5, CV_64FC1);
			pK->v("k1", &mD.at<double>(0, 0));
			pK->v("k2", &mD.at<double>(0, 1));
			pK->v("p1", &mD.at<double>(0, 2));
			pK->v("p2", &mD.at<double>(0, 3));
			pK->v("k3", &mD.at<double>(0, 4));

			setCamMatrices(mC, mD);
		}
		DEL(pKf);

		return true;
	}

	Frame *_VisionBase::BGR(void)
	{
		return &m_fBGR;
	}

	vInt2 _VisionBase::getSize(void)
	{
		return m_vSize;
	}

	VISION_TYPE _VisionBase::getType(void)
	{
		return m_type;
	}

	bool _VisionBase::open(void)
	{
		return true;
	}

	bool _VisionBase::isOpened(void)
	{
		return m_bOpen;
	}

	void _VisionBase::close(void)
	{
		IF_(check() < 0);

		m_pT->goSleep();
		while (!m_pT->bSleeping())
			;

		m_bOpen = false;
	}

	bool _VisionBase::setCamMatrices(const Mat &mC, const Mat &mD)
	{
		IF_F(mC.empty() || mD.empty());

		m_mC = mC;
		m_mD = mD;
		return scaleCamMatrices();
	}

	bool _VisionBase::scaleCamMatrices(void)
	{
		IF_F(m_mC.empty() || m_mD.empty());

		cv::Size s(m_vSize.x, m_vSize.y);
		Mat mCs;
		m_mC.copyTo(mCs);
		mCs.at<double>(0, 0) *= (double)s.width;  //Fx
		mCs.at<double>(1, 1) *= (double)s.height; //Fy
		mCs.at<double>(0, 2) *= (double)s.width;  //Cx
		mCs.at<double>(1, 2) *= (double)s.height; //Cy

		m_mCscaled = getOptimalNewCameraMatrix(mCs, m_mD, s, 1, s, 0);
		initUndistortRectifyMap(mCs, m_mD, Mat(), m_mCscaled, s, CV_16SC2, m_m1, m_m2);

		return true;
	}

	vDouble2 _VisionBase::getF(void)
	{
		vDouble2 vF;
		vF.x = m_mCscaled.at<double>(0, 0);
		vF.y = m_mCscaled.at<double>(1, 1);
		return vF;
	}

	vDouble2 _VisionBase::getC(void)
	{
		vDouble2 vC;
		vC.x = m_mCscaled.at<double>(0, 2);
		vC.y = m_mCscaled.at<double>(1, 2);
		return vC;
	}

	vFloat2 _VisionBase::getFf(void)
	{
		vFloat2 vF;
		vF.x = (float)m_mCscaled.at<double>(0, 0);
		vF.y = (float)m_mCscaled.at<double>(1, 1);
		return vF;
	}

	vFloat2 _VisionBase::getCf(void)
	{
		vFloat2 vC;
		vC.x = (float)m_mCscaled.at<double>(0, 2);
		vC.y = (float)m_mCscaled.at<double>(1, 2);
		return vC;
	}

	Mat _VisionBase::mC(void)
	{
		return m_mC;
	}

	Mat _VisionBase::mCscaled(void)
	{
		return m_mCscaled;
	}

	Mat _VisionBase::mD(void)
	{
		return m_mD;
	}

	void _VisionBase::cvDraw(void *pWindow)
	{
		NULL_(pWindow);
		this->_ModuleBase::cvDraw(pWindow);
		IF_(check() < 0);

		_WindowCV *pWin = (_WindowCV *)pWindow;
		Frame *pF = pWin->getFrame();
		NULL_(pF);
		Mat *pM = pF->m();
		IF_(pM->empty());

		IF_(m_fBGR.bEmpty());

		if (m_bbDraw.x < 0.0)
		{
			pF->copy(m_fBGR);
		}
		else
		{
			Rect r = bb2Rect(bbScale(m_bbDraw, pM->cols, pM->rows));

			Mat m;
			cv::resize(*m_fBGR.m(), m, Size(r.width, r.height));

			m.copyTo((*pM)(r));
		}
	}

}
#endif
