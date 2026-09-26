/*
 * _VideoFile.cpp
 *
 *  Created on: Aug 5, 2018
 *      Author: yankai
 */

#include "_VideoFile.h"

namespace kai
{

	_VideoFile::_VideoFile()
	{
		m_type = vision_file;
	}

	_VideoFile::~_VideoFile()
	{
	}

	bool _VideoFile::loadConfig(void)
	{
		IF_F(!_VisionBase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "videoFile", m_videoFile);

		return true;
	}

	bool _VideoFile::open(void)
	{
		m_vc.open(m_videoFile);
		if (!m_vc.isOpened())
		{
			LOG_E("Cannot open file: " + m_videoFile);
			return false;
		}

		m_vc.set(CAP_PROP_FRAME_WIDTH, m_vSizeRGB.x());
		m_vc.set(CAP_PROP_FRAME_HEIGHT, m_vSizeRGB.y());
		m_vc.set(CAP_PROP_FPS, m_pT->getTargetFPS());

		Mat cMat;
		while (!m_vc.read(cMat))
			;

		m_vSizeRGB.x() = cMat.cols;
		m_vSizeRGB.y() = cMat.rows;

		m_bOpened = true;
		return true;
	}

	void _VideoFile::close(void)
	{
		this->_VisionBase::close();
		m_vc.release();
	}

	bool _VideoFile::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _VideoFile::update(void)
	{
		while (m_pT->bRun())
		{
			if (!m_bOpened)
			{
				if (!open())
				{
					m_pT->sleepT(NSEC_SEC);
					continue;
				}
			}

			m_pT->autoFPS();

			Mat mCam;
			while (!m_vc.read(mCam))
				;
			std::lock_guard<std::mutex> lock(m_mutexRGB);
			mCam.copyTo(m_mRGB);
		}
	}

}
