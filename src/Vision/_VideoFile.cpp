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
	m_vc.release();
}

bool _VideoFile::init(void* pKiss)
{
	IF_F(!_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, videoFile);

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

	m_vc.set(CAP_PROP_FRAME_WIDTH, m_w);
	m_vc.set(CAP_PROP_FRAME_HEIGHT, m_h);
	m_vc.set(CAP_PROP_FPS, m_targetFPS);

	Mat cMat;
	while (!m_vc.read(cMat));

	m_w = cMat.cols;
	m_h = cMat.rows;

	m_cW = m_w / 2;
	m_cH = m_h / 2;

	m_bOpen = true;
	return true;
}

bool _VideoFile::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _VideoFile::update(void)
{
	while (m_bThreadON)
	{
		if (!m_bOpen)
		{
			if (!open())
			{
				this->sleepTime(USEC_1SEC);
				continue;
			}
		}

		this->autoFPSfrom();

		Mat mCam;
		while (!m_vc.read(mCam));
		m_fBGR.copy(mCam);

		this->autoFPSto();
	}
}

}
