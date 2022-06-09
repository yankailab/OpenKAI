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
		m_gstOutput = "appsrc ! videoconvert ! fbdevsink";
		m_vSize.set(1280, 720);
	}

	_GstOutput::~_GstOutput()
	{
	}

	bool _GstOutput::init(void *pKiss)
	{
		IF_F(!this->_UIbase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vSize", &m_vSize);
		IF_F(m_vSize.area() <= 0);

		m_F.allocate(m_vSize.x, m_vSize.y);
		*m_F.m() = Scalar(0, 0, 0);

		pK->v("gstOutput", &m_gstOutput);
		if (!m_gstOutput.empty())
		{
			int fps = (int)m_pT->getTargetFPS();
			if (!m_gst.open(m_gstOutput,
							CAP_GSTREAMER,
							0,
							fps,
							cv::Size(m_vSize.x, m_vSize.y),
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
		return m_pT->start(getUpdate, this);
	}

	void _GstOutput::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			updateGst();

			m_pT->autoFPSto();
		}
	}

	void _GstOutput::updateGst(void)
	{
		IF_(!m_gst.isOpened());

		// draw contents
		for (BASE *pB : m_vpB)
		{
			pB->draw((void*)&m_F);
		}

		Size fs = m_F.size();
		if (fs.width != m_vSize.x || fs.height != m_vSize.y)
		{
			Frame F;
			F.copy(m_F);
			m_F = F.resize(m_vSize.x, m_vSize.y);
		}

		m_gst << *m_F.m();
	}
}
