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
		m_videoFile = "";
	}

	_VideoFile::~_VideoFile()
	{
	}

	bool _VideoFile::init(const json &j)
	{
		IF_F(!_VisionBase::init(j));

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

		m_vc.set(CAP_PROP_FRAME_WIDTH, m_vSizeRGB.x);
		m_vc.set(CAP_PROP_FRAME_HEIGHT, m_vSizeRGB.y);
		m_vc.set(CAP_PROP_FPS, m_pT->getTargetFPS());

		Mat cMat;
		while (!m_vc.read(cMat))
			;

		m_vSizeRGB.x = cMat.cols;
		m_vSizeRGB.y = cMat.rows;

		m_bOpen = true;
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
		return m_pT->start(getUpdate, this);
	}

	void _VideoFile::update(void)
	{
		while (m_pT->bAlive())
		{
			if (!m_bOpen)
			{
				if (!open())
				{
					m_pT->sleepT(SEC_2_USEC);
					continue;
				}
			}

			m_pT->autoFPS();

			Mat mCam;
			while (!m_vc.read(mCam))
				;
			m_fRGB.copy(mCam);
		}
	}

}
