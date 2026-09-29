/*
 * Window.cpp
 *
 *  Created on: Dec 7, 2016
 *      Author: Kai Yan
 */

#include "_WindowCV.h"

namespace kai
{

	_WindowCV::_WindowCV()
	{
		m_vSize = Vector2i(1280, 720);
	}

	_WindowCV::~_WindowCV()
	{
	}

	bool _WindowCV::loadConfig(void)
	{
		IF_F(!this->_UIbase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "bFullScreen", m_bFullScreen);
		jKv<int>(j, "vSize", m_vSize);

		IF_Le_F(m_vSize.x() <= 0 || m_vSize.y() <= 0, "Window size too small");

		string wn = this->getName();
		if (m_bFullScreen)
		{
			namedWindow(wn, WINDOW_NORMAL);
			setWindowProperty(wn, WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);
		}
		else
		{
			namedWindow(wn, WINDOW_AUTOSIZE);
		}

		return true;
	}

	bool _WindowCV::saveConfig(bool bExport)
	{
		IF_F(!_UIbase::saveConfig(false));

		json &j = *m_pJ;
		j["bFullScreen"] = m_bFullScreen;
		j["vSize"] = {m_vSize.x(), m_vSize.y()};

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _WindowCV::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _WindowCV::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			updateWindow();
		}
	}

	void _WindowCV::updateWindow(void)
	{
		NULL_(m_pRGBin);
		Mat input;
		m_pRGBin->get(input);
		const Mat image = prepareImage(input, m_vSize);
		IF_(image.empty());

		imshow(this->getName(), image);

		// autoFPS() controls the refresh rate. Keep event handling short because
		// HighGUI serializes these calls across all preview windows.
		waitKey(1);
	}
}
