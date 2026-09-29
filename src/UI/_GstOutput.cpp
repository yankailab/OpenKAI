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

	bool _GstOutput::loadConfig(void)
	{
		IF_F(!this->_UIbase::loadConfig());
		const json &j = *m_pJ;

		jKv<int>(j, "vSize", m_vSize);
		IF_Le_F(m_vSize.x() <= 0 || m_vSize.y() <= 0, "Invalid GStreamer output size");

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

	bool _GstOutput::saveConfig(bool bExport)
	{
		IF_F(!_UIbase::saveConfig(false));

		json &j = *m_pJ;
		j["vSize"] = {m_vSize.x(), m_vSize.y()};
		j["gstOutput"] = m_gstOutput;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
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

		NULL_(m_pRGBin);
		Mat input;
		m_pRGBin->get(input);
		const Mat image = prepareImage(input, m_vSize);
		IF_(image.empty());

		m_gst << image;
	}
}
