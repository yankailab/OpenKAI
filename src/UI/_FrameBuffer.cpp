/*
 * Window.cpp
 *
 *  Created on: May 24, 2022
 *      Author: Kai Yan
 */

#include "_FrameBuffer.h"

namespace kai
{

	_FrameBuffer::_FrameBuffer()
	{
		m_gstOutput = "appsrc ! videoconvert ! fbdevsink";
		m_vSize.set(1280, 720);
	}

	_FrameBuffer::~_FrameBuffer()
	{
	}

	bool _FrameBuffer::init(void *pKiss)
	{
		IF_F(!this->_UIbase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vSize", &m_vSize);
		if (m_vSize.area() > 0)
		{
			m_sF.get()->allocate(m_vSize.x, m_vSize.y);
			m_sF.next()->allocate(m_vSize.x, m_vSize.y);
		}

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

	bool _FrameBuffer::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _FrameBuffer::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			updateFB();

			m_pT->autoFPSto();
		}
	}

	void _FrameBuffer::updateFB(void)
	{
		IF_(!m_gst.isOpened());

		// draw contents
		for (BASE *pB : m_vpB)
		{
			pB->draw((void*)m_sF.get());
		}

		IF_(m_sF.next()->bEmpty());
		m_sF.swap();
		*m_sF.next()->m() = Scalar(0, 0, 0);

		Frame F, F2;
		F.copy(*m_sF.get());
		Size fs = F.size();

		if (fs.width != m_vSize.x || fs.height != m_vSize.y)
		{
			F2 = F.resize(m_vSize.x, m_vSize.y);
			F = F2;
		}

		m_gst << *F.m();
	}
}
