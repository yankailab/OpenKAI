/*
 * _Remap.cpp
 *
 *  Created on: May 7, 2021
 *      Author: yankai
 */

#include "_Remap.h"

#ifdef USE_OPENCV

namespace kai
{

	_Remap::_Remap()
	{
		m_type = vision_remap;
		m_pV = NULL;
		m_bReady = false;
		m_yml = "";
	}

	_Remap::~_Remap()
	{
		close();
	}

	bool _Remap::init(void *pKiss)
	{
		IF_F(!_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("yml", &m_yml);
		if (!m_yml.empty())
		{
			FileStorage fs(m_yml.c_str(), FileStorage::READ);
			if (fs.isOpened())
			{
				fs["mC"] >> m_mC;
				fs["mD"] >> m_mD;
				fs.release();
			}
		}

		string n;
		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->getInst(n));
		IF_Fl(!m_pV, n + ": not found");

		return true;
	}

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

	void _Remap::filter(void)
	{
		Frame *pF = m_pV->BGR();
		IF_(pF->bEmpty());
		IF_(m_fBGR.tStamp() >= pF->tStamp())

		if(!m_bReady)
		{
			m_fBGR.copy(*pF);
			m_bReady = setup();
			IF_(!m_bReady);
		}

		if(m_fBGR.size() != m_s)
		{
			m_bReady = setup();
			IF_(!m_bReady);
		}

		m_fBGR.copy(pF->remap(m_m1, m_m2));
	}

	bool _Remap::setup(void)
	{
		IF_F(m_mC.empty() || m_mD.empty());

		return setCamMatrices(m_mC, m_mD);
	}

	bool _Remap::setCamMatrices(const Mat &mC, const Mat &mD)
	{
		IF_F(m_fBGR.bEmpty());

		m_mC = mC;
		m_mD = mD;
		m_s = m_fBGR.size();

		Mat mCs;
		m_mC.copyTo(mCs);
		mCs.at<double>(0,0) *= (double)m_s.width;  //Fx
		mCs.at<double>(1,1) *= (double)m_s.height; //Fy
		mCs.at<double>(0,2) *= (double)m_s.width;  //Cx
		mCs.at<double>(1,2) *= (double)m_s.height; //Cy

		m_mCscaled = getOptimalNewCameraMatrix(mCs, m_mD, m_s, 1, m_s, 0);
		initUndistortRectifyMap(mCs, m_mD, Mat(), m_mCscaled, m_s, CV_16SC2, m_m1, m_m2);

		return true;
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
#endif
