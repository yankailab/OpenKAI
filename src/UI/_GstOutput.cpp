/*
 * Window.cpp
 *
 *  Created on: May 24, 2022
 *      Author: Kai Yan
 */

#include "_GstOutput.h"

namespace kai
{

	_GstOutput::_GstOutput()
	{
		m_vSize = Vector2i(1280, 720);
	}

	_GstOutput::~_GstOutput()
	{
	}

	bool _GstOutput::init(const json &j)
	{
		IF_F(!this->_UIbase::init(j));

		jKv<int>(j, "vSize", m_vSize);
		IF_F(std::abs(m_vSize.prod()) <= 0);

		m_M = Mat::zeros(m_vSize.y(), m_vSize.x(), CV_8UC3);

		jKv(j, "gstOutput", m_gstOutput);
		if (!m_gstOutput.empty())
		{
			if (!m_gst.open(m_gstOutput,
							CAP_GSTREAMER,
							0,
							m_pT->getTargetFPS(),
							cv::Size(m_vSize.x(), m_vSize.y()),
							true))
			{
				LOG_E("Cannot open GStreamer output");
				return false;
			}
		}

		return true;
	}

	bool _GstOutput::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _GstOutput::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			updateGst();
		}
	}

	void _GstOutput::updateGst(void)
	{
		IF_(!m_gst.isOpened());

		// draw contents
		m_M = Scalar(0);
		for (BASE *pB : m_vpB)
		{
			pB->draw((void *)&m_M);
		}

		Size fs = m_M.size();
		if (fs.width != m_vSize.x() || fs.height != m_vSize.y())
		{
			cv::resize(m_M, m_M, cv::Size(m_vSize.x(), m_vSize.y()));
		}

		Mat m = m_M;
		if (m.type() != CV_8UC3)
		{
			cv::cvtColor(m_M, m, COLOR_GRAY2BGR);
		}

		m_gst << m;
	}
}
