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
		FileStorage fs(path.c_str(), FileStorage::READ);
		IF_F(!fs.isOpened());

		fs["mCam"] >> m_mCam;
		fs["mDistCoeffs"] >> m_mDistCoeffs;
		fs.release();

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

				if (m_bOpen)
				{
					m_pV->BGR()->setRemap(m_mX, m_mY);
				}
				else
				{
					continue;
				}
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
		IF_(m_fBGR.tStamp() < pF->tStamp())

		m_fBGR.copy(pF->remap());
	}

	void _Remap::setCamMatrices(const Mat &mCam, const Mat &mDistCoeffs)
	{
		NULL_(!m_pV);
		IF_(m_fBGR.bEmpty());

		m_mCam = mCam;
		m_mDistCoeffs = mDistCoeffs;

		m_mCamNew = getOptimalNewCameraMatrix(mCam, m_mDistCoeffs, m_fBGR.size(), 1, m_fBGR.size(), 0);
		initUndistortRectifyMap(mCam, m_mDistCoeffs, Mat(), m_mCamNew, m_fBGR.size(), CV_16SC2, m_mX, m_mY);

		m_pV->BGR()->setRemap(m_mX, m_mY);
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
