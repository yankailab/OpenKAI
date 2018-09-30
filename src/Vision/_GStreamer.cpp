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
}

_GStreamer::~_GStreamer()
{
	m_gst.release();
}

bool _GStreamer::init(void* pKiss)
{
	IF_F(!_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, pipeline);
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

	Mat cMat;
	//Acquire a frame to determine the actual frame size
	while (!m_gst.read(cMat));

	m_w = cMat.cols;
	m_h = cMat.rows;

	m_cW = m_w / 2;
	m_cH = m_h / 2;

	m_bOpen = true;
	return true;
}

void _GStreamer::close(void)
{
	goSleep();
	while(!bSleeping());

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
