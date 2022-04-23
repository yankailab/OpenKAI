/*
 * _Remap.cpp
 *
 *  Created on: May 7, 2021
 *      Author: yankai
 */

#include "_Remap.h"

namespace kai
{

	_Remap::_Remap()
	{
		m_type = vision_remap;
		m_pV = NULL;
		m_bReady = false;
		m_fCalib ="";
	}

	_Remap::~_Remap()
	{
		close();
	}

	bool _Remap::init(void *pKiss)
	{
		IF_F(!_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		string n;
		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->getInst(n));
		IF_Fl(!m_pV, n + ": not found");

		pK->v("fCalib", &m_fCalib);
		Mat mC, mD;
		IF_F(!readCamMatrices(m_fCalib, &mC, &mD));
		m_bReady = setCamMat(mC, mD);

		return true;
	}

	// void _Remap::updateCamMat(void)
	// {
	// 	Kiss *pKf = new Kiss();
	// 	if (parseKiss(m_fCalib, pKf))
	// 	{
	// 		Kiss* pK = pKf->child("calib");
	// 		IF_d_(pK->empty(), DEL(pKf));

	// 		Mat mC = Mat::zeros(3, 3, CV_64FC1);
	// 		pK->v("Fx", &mC.at<double>(0, 0));
	// 		pK->v("Fy", &mC.at<double>(1, 1));
	// 		pK->v("Cx", &mC.at<double>(0, 2));
	// 		pK->v("Cy", &mC.at<double>(1, 2));
	// 		mC.at<double>(2, 2) = 1.0;

	// 		Mat mD = Mat::zeros(1, 5, CV_64FC1);
	// 		pK->v("k1", &mD.at<double>(0, 0));
	// 		pK->v("k2", &mD.at<double>(0, 1));
	// 		pK->v("p1", &mD.at<double>(0, 2));
	// 		pK->v("p2", &mD.at<double>(0, 3));
	// 		pK->v("k3", &mD.at<double>(0, 4));

	// 		m_bReady = setCamMatrices(mC, mD);
	// 	}
	// 	DEL(pKf);
	// }

	bool _Remap::open(void)
	{
		NULL_F(m_pV);
		m_bOpen = m_pV->isOpened();

		return m_bOpen;
	}

	void _Remap::close(void)
	{
		this->_VisionBase::close();
	}

	bool _Remap::bReady(void)
	{
		return m_bReady;
	}

	bool _Remap::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _Remap::update(void)
	{
		while (m_pT->bRun())
		{
			if (!m_bOpen)
			{
				open();
				if (!m_bOpen)
					continue;
			}

			m_pT->autoFPSfrom();

			filter();

			m_pT->autoFPSto();
		}
	}

	bool _Remap::setCamMat(const Mat &mC, const Mat &mD)
	{
		IF_F(mC.empty() || mD.empty());

		m_mC = mC;
		m_mD = mD;
		return scaleCamMat();
	}

	bool _Remap::scaleCamMat(void)
	{
		cv::Size s(m_vSize.x, m_vSize.y);
		IF_F(!scaleCamMatrices(s,
							  m_mC,
							  m_mD,
							  &m_mCscaled));

#ifdef USE_CUDA
		initUndistortRectifyMap(m_mCscaled, m_mD, Mat(), m_mCscaled, s, CV_32F, m_m1, m_m2);
#else
		initUndistortRectifyMap(m_mCscaled, m_mD, Mat(), m_mCscaled, s, CV_16SC2, m_m1, m_m2);
#endif

		return true;

		// IF_F(m_mC.empty() || m_mD.empty());
		// cv::Size s(m_vSize.x, m_vSize.y);
		// Mat mCs;
		// m_mC.copyTo(mCs);
		// mCs.at<double>(0, 0) *= (double)s.width;  //Fx
		// mCs.at<double>(1, 1) *= (double)s.height; //Fy
		// mCs.at<double>(0, 2) *= (double)s.width;  //Cx
		// mCs.at<double>(1, 2) *= (double)s.height; //Cy
		// m_mCscaled = getOptimalNewCameraMatrix(mCs, m_mD, s, 1, s, 0);
		// return true;
	}

	vDouble2 _Remap::getF(void)
	{
		vDouble2 vF = {0,0};
		IF__(m_mCscaled.empty(), vF);

		vF.x = m_mCscaled.at<double>(0, 0);
		vF.y = m_mCscaled.at<double>(1, 1);
		return vF;
	}

	vDouble2 _Remap::getC(void)
	{
		vDouble2 vC = {0,0};
		IF__(m_mCscaled.empty(), vC);

		vC.x = m_mCscaled.at<double>(0, 2);
		vC.y = m_mCscaled.at<double>(1, 2);
		return vC;
	}

	vFloat2 _Remap::getFf(void)
	{
		vFloat2 vF = {0,0};
		IF__(m_mCscaled.empty(), vF);

		vF.x = (float)m_mCscaled.at<double>(0, 0);
		vF.y = (float)m_mCscaled.at<double>(1, 1);
		return vF;
	}

	vFloat2 _Remap::getCf(void)
	{
		vFloat2 vC = {0,0};
		IF__(m_mCscaled.empty(), vC);

		vC.x = (float)m_mCscaled.at<double>(0, 2);
		vC.y = (float)m_mCscaled.at<double>(1, 2);
		return vC;
	}

	Mat _Remap::mC(void)
	{
		return m_mC;
	}

	Mat _Remap::mCscaled(void)
	{
		return m_mCscaled;
	}

	Mat _Remap::mD(void)
	{
		return m_mD;
	}

	void _Remap::filter(void)
	{
		Frame *pF = m_pV->BGR();
		IF_(pF->bEmpty());
		IF_(m_fBGR.tStamp() >= pF->tStamp())

		if(!m_bReady || pF->size() != cv::Size(m_vSize.x, m_vSize.y))
		{
			cv::Size s = pF->size();
			m_vSize.x = s.width;
			m_vSize.y = s.height;
			m_bReady = scaleCamMat();
		}

		if(m_bReady)
			m_fBGR.copy(pF->remap(m_m1, m_m2));
		else
			m_fBGR.copy(*pF);
	}
}
