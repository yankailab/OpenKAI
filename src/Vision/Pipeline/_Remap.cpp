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
	}

	_Remap::~_Remap()
	{
	}

	bool _Remap::loadConfig(void)
	{
		IF_F(!_VisionBase::loadConfig());

		return true;
	}

	bool _Remap::saveConfig(bool bExport)
	{
		if (!_VisionBase::saveConfig(false))
		{
			return false;
		}

		if (!bExport)
		{
			return true;
		}
		return m_pJcfg->saveToFile();
	}

	bool _Remap::link(void)
	{
		IF_F(!this->_VisionBase::link());
		const json &j = *m_pJ;

		string n;

		n = "";
		jKv(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(m_pM->findModule(n));
		NULL_F(m_pV);

		n = "";
		jKv(j, "fCalib", m_fCalib);
		Mat mC, mD;
		IF_F(!readCamMatrices(m_fCalib, &mC, &mD));
		m_bReady = setCamMat(mC, mD);

		return true;
	}

	bool _Remap::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _Remap::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _Remap::filter(void)
	{
		NULL_(m_pV);
		Mat mIn;
		m_pV->copyMatRGB(mIn);
		IF_(mIn.empty());

		if (!m_bReady || mIn.size() != cv::Size(m_vSizeRGB.x(), m_vSizeRGB.y()))
		{
			cv::Size s = mIn.size();
			m_vSizeRGB.x() = s.width;
			m_vSizeRGB.y() = s.height;
			m_bReady = scaleCamMat();
		}

		std::lock_guard<std::mutex> lock(m_mutexRGB);
		if (m_bReady)
			cv::remap(mIn, m_mRGB, m_m1, m_m2, cv::INTER_LINEAR);
		else
			mIn.copyTo(m_mRGB);
	}

	// void _Remap::updateCamMat(void)
	// {
	// 	Kiss *pKf = new Kiss();
	// 	if (parseKiss(m_fCalib, pKf))
	// 	{
	// 		Kiss* pK = pKf->child("calib");
	// 		IF_d_(pK->empty(), DEL(pKf));

	// 		Mat mC = Mat::zeros(3, 3, CV_64FC1);
	// 		= j.value("Fx", &mC.at<double>(0, 0));
	// 		= j.value("Fy", &mC.at<double>(1, 1));
	// 		= j.value("Cx", &mC.at<double>(0, 2));
	// 		= j.value("Cy", &mC.at<double>(1, 2));
	// 		mC.at<double>(2, 2) = 1.0;

	// 		Mat mD = Mat::zeros(1, 5, CV_64FC1);
	// 		= j.value("k1", &mD.at<double>(0, 0));
	// 		= j.value("k2", &mD.at<double>(0, 1));
	// 		= j.value("p1", &mD.at<double>(0, 2));
	// 		= j.value("p2", &mD.at<double>(0, 3));
	// 		= j.value("k3", &mD.at<double>(0, 4));

	// 		m_bReady = setCamMatrices(mC, mD);
	// 	}
	// 	DEL(pKf);
	// }

	bool _Remap::setCamMat(const Mat &mC, const Mat &mD)
	{
		IF_F(mC.empty() || mD.empty());

		m_mC = mC;
		m_mD = mD;
		return scaleCamMat();
	}

	bool _Remap::scaleCamMat(void)
	{
		cv::Size s(m_vSizeRGB.x(), m_vSizeRGB.y());
		IF_F(!scaleCamMatrices(s,
							   m_mC,
							   m_mD,
							   &m_mCscaled));

		initUndistortRectifyMap(m_mCscaled, m_mD, Mat(), m_mCscaled, s, CV_16SC2, m_m1, m_m2);

		return true;

		// IF_F(m_mC.empty() || m_mD.empty());
		// cv::Size s(m_vSize.x(), m_vSize.y());
		// Mat mCs;
		// m_mC.copyTo(mCs);
		// mCs.at<double>(0, 0) *= (double)s.width;  //Fx
		// mCs.at<double>(1, 1) *= (double)s.height; //Fy
		// mCs.at<double>(0, 2) *= (double)s.width;  //Cx
		// mCs.at<double>(1, 2) *= (double)s.height; //Cy
		// m_mCscaled = getOptimalNewCameraMatrix(mCs, m_mD, s, 1, s, 0);
		// return true;
	}

	Vector2d _Remap::getF(void)
	{
		Vector2d vF = {0, 0};
		IF__(m_mCscaled.empty(), vF);

		vF.x() = m_mCscaled.at<double>(0, 0);
		vF.y() = m_mCscaled.at<double>(1, 1);
		return vF;
	}

	Vector2d _Remap::getC(void)
	{
		Vector2d vC = {0, 0};
		IF__(m_mCscaled.empty(), vC);

		vC.x() = m_mCscaled.at<double>(0, 2);
		vC.y() = m_mCscaled.at<double>(1, 2);
		return vC;
	}

	Vector2f _Remap::getFf(void)
	{
		Vector2f vF = {0, 0};
		IF__(m_mCscaled.empty(), vF);

		vF.x() = (float)m_mCscaled.at<double>(0, 0);
		vF.y() = (float)m_mCscaled.at<double>(1, 1);
		return vF;
	}

	Vector2f _Remap::getCf(void)
	{
		Vector2f vC = {0, 0};
		IF__(m_mCscaled.empty(), vC);

		vC.x() = (float)m_mCscaled.at<double>(0, 2);
		vC.y() = (float)m_mCscaled.at<double>(1, 2);
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
}
