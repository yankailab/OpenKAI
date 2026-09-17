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

	bool _WindowCV::init(const json &j)
	{
		IF_F(!this->_UIbase::init(j));

		jKv(j, "bFullScreen", m_bFullScreen);
		jKv<int>(j, "vSize", m_vSize);
		m_waitKey = 1000.0f / m_pT->getTargetFPS();

		IF_Le_F(std::abs(m_vSize.prod()) <= 0, "Window size too small");
		m_M = Mat::zeros(m_vSize.y(), m_vSize.x(), CV_8UC3);

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
		// draw contents
		for (BASE *pB : m_vpB)
		{
			pB->draw((void *)&m_M);
		}
		IF_(m_M.empty());

		// show window
		imshow(this->getName(), m_M);

		// int key = waitKey(m_waitKey);
		waitKey(m_waitKey);
	}
}
