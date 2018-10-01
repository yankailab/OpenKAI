/*
 * _Gstreamer.cpp
 *
 *  Created on: Dec 21, 2017
 *      Author: yankai
 */


#include "_GStreamer.h"

namespace kai
{

_GStreamer::_GStreamer()
{
	m_type = vision_gstreamer;
	m_pipeline = "";
	m_nInitRead = 1;
}

_GStreamer::~_GStreamer()
{
	close();
}

bool _GStreamer::init(void* pKiss)
{
	IF_F(!_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, pipeline);
	KISSm(pK, nInitRead);

	return true;
}

bool _GStreamer::open(void)
{
	m_gst.open(m_pipeline, CAP_GSTREAMER);
	if (!m_gst.isOpened())
	{
		LOG_E("Cannot open gst pipeline: " + m_pipeline);
		return false;
	}

	Mat mCam;
	for(int i=0; i<m_nInitRead; i++)
	{
		while (!m_gst.read(mCam));
	}
	m_fBGR.copy(mCam);

	m_w = mCam.cols;
	m_h = mCam.rows;

	m_cW = m_w / 2;
	m_cH = m_h / 2;

	m_bOpen = true;
	return true;
}

void _GStreamer::close(void)
{
	if(m_threadMode==T_THREAD)
	{
		goSleep();
		while(!bSleeping());
	}

	m_gst.release();
	this->_VisionBase::close();
}

bool _GStreamer::start(void)
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

void _GStreamer::update(void)
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
		while (!m_gst.read(mCam));
		m_fBGR.copy(mCam);

		this->autoFPSto();
	}
}

}
