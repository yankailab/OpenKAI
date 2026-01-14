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
	}

	_GStreamer::~_GStreamer()
	{
		close();
	}

	bool _GStreamer::init(const json &j)
	{
		IF_F(!_VisionBase::init(j));

		m_pipeline = j.value("pipeline", "");
		m_nInitRead = j.value("nInitRead", 1);

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
		for (int i = 0; i < m_nInitRead; i++)
		{
			while (!m_gst.read(mCam))
				;
		}
		m_fRGB.copy(mCam);

		m_vSizeRGB.x = mCam.cols;
		m_vSizeRGB.y = mCam.rows;

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
			while (!m_gst.read(mCam))
				;
			m_fRGB.copy(mCam);
		}
	}

}
