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

	int _GstOutput::init(void *pKiss)
	{
		CHECK_(this->_UIbase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vSize", &m_vSize);
		IF__(m_vSize.area() <= 0, OK_ERR_INVALID_VALUE);

		m_F.allocate(m_vSize.x, m_vSize.y);
		*m_F.m() = Scalar(0, 0, 0);

		pK->v("gstOutput", &m_gstOutput);
		if (!m_gstOutput.empty())
		{
			if (!m_gst.open(m_gstOutput,
							CAP_GSTREAMER,
							0,
							m_pT->getTargetFPS(),
							cv::Size(m_vSize.x, m_vSize.y),
							true))
			{
				LOG_E("Cannot open GStreamer output");
				return OK_ERR_UNKNOWN;
			}
		}

		return OK_OK;
	}

	int _GstOutput::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	void _GstOutput::update(void)
	{
		while (m_pT->bAlive())
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
		*m_F.m() = Scalar(0);
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

		Mat m = *m_F.m();
		if(m.type() != CV_8UC3)
		{
			cv::cvtColor(*m_F.m(), m, COLOR_GRAY2BGR);
		}

		m_gst << m;
	}
}
