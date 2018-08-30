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
	pK->m_pInst = this;

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

	m_vc.set(CV_CAP_PROP_FRAME_WIDTH, m_w);
	m_vc.set(CV_CAP_PROP_FRAME_HEIGHT, m_h);
	m_vc.set(CV_CAP_PROP_FPS, m_targetFPS);

	Mat cMat;
	//Acquire a frame to determine the actual frame size
	while (!m_vc.read(cMat));

	m_w = cMat.cols;
	m_h = cMat.rows;

	if (m_bCrop)
	{
		int i;
		i = cMat.cols - m_cropBB.x;
		if (m_cropBB.width > i)
			m_cropBB.width = i;

		i = cMat.rows - m_cropBB.y;
		if (m_cropBB.height > i)
			m_cropBB.height = i;

		m_w = m_cropBB.width;
		m_h = m_cropBB.height;
	}

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

	retCode = pthread_create(&m_pTPP->m_threadID, 0, getTPP, this);
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
		m_pTPP->wakeUp();

		this->autoFPSto();
	}
}

void _VideoFile::updateTPP(void)
{
	while (m_bThreadON)
	{
		m_pTPP->sleepTime(0);
		postProcess();
	}
}

}
