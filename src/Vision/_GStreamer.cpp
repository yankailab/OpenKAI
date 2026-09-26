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

	bool _GStreamer::loadConfig(void)
	{
		IF_F(!_VisionBase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "pipeline", m_pipeline);
		jKv(j, "nInitRead", m_nInitRead);

		return true;
	}

	bool _GStreamer::saveConfig(bool bExport)
	{
		if (!_VisionBase::saveConfig(false))
		{
			return false;
		}

		json &j = *m_pJ;
		j["pipeline"] = m_pipeline;
		j["nInitRead"] = m_nInitRead;

		if (!bExport)
		{
			return true;
		}
		return m_pJcfg->saveToFile();
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
		
		{
			std::lock_guard<std::mutex> lock(m_mutexRGB);
			mCam.copyTo(m_mRGB);
		}

		m_vSizeRGB.x() = mCam.cols;
		m_vSizeRGB.y() = mCam.rows;

		m_bOpened = true;
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
		return m_pT->startThread(getUpdate, this);
	}

	void _GStreamer::update(void)
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
			while (!m_gst.read(mCam))
				;
			std::lock_guard<std::mutex> lock(m_mutexRGB);
			mCam.copyTo(m_mRGB);
		}
	}

}
