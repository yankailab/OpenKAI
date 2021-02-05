/*
 * _Gstreamer.cpp
 *
 *  Created on: Dec 21, 2017
 *      Author: yankai
 */

#include "_GStreamer.h"

#ifdef USE_OPENCV

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

	pK->v("pipeline",&m_pipeline);
	pK->v("nInitRead",&m_nInitRead);

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

	m_vSize.x = mCam.cols;
	m_vSize.y = mCam.rows;

	m_bOpen = true;
	return true;
}

void _GStreamer::close(void)
{
	this->_VisionBase::close();
	m_gst.release();
}

bool _GStreamer::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

void _GStreamer::update(void)
{
	while(m_pT->bRun())
	{
		if (!m_bOpen)
		{
			if (!open())
			{
				m_pT->sleepTime(USEC_1SEC);
				continue;
			}
		}

		m_pT->autoFPSfrom();

		Mat mCam;
		while (!m_gst.read(mCam));
		m_fBGR.copy(mCam);

		m_pT->autoFPSto();
	}
}

}
#endif
