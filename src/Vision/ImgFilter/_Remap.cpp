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

		m_mCam = Mat::eye(3, 3, CV_64FC1);
		m_mCamNew = m_mCam;
		m_mDistCoeffs = Mat::zeros(1, 5, CV_64FC1);
	}

	_Remap::~_Remap()
	{
		close();
	}

	bool _Remap::init(void *pKiss)
	{
		IF_F(!_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		string path;
		pK->v("yml", &path);
		if (!path.empty())
		{
			FileStorage fs(path.c_str(), FileStorage::READ);
			if (fs.isOpened())
			{
				fs["mCam"] >> m_mCam;
				fs["mDistCoeffs"] >> m_mDistCoeffs;
				fs.release();
			}
		}

		double t;
		pK->v("test", &t);
		m_mCamNew *= t;
//		setCamMatrices(m_mCam, m_mDistCoeffs);

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

//		m_fBGR.copy(pF->remap(m_mX, m_mY));
		m_fBGR.copy(*pF);
	}

	void _Remap::setCamMatrices(const Mat &mCam, const Mat &mDistCoeffs)
	{
		m_mCam = mCam;
		m_mDistCoeffs = mDistCoeffs;

		m_mCamNew = getOptimalNewCameraMatrix(mCam, m_mDistCoeffs, m_fBGR.size(), 1, m_fBGR.size(), 0);
		initUndistortRectifyMap(mCam, m_mDistCoeffs, Mat(), m_mCamNew, m_fBGR.size(), CV_16SC2, m_mX, m_mY);
	}

	Mat _Remap::mC(void)
	{
		return m_mCamNew;
	}

	Mat _Remap::mD(void)
	{
		return m_mDistCoeffs;
	}

}
#endif
