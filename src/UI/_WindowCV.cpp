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
		m_waitKey = 30;
		m_bFullScreen = false;
		m_vSize.set(1280, 720);
	}

	_WindowCV::~_WindowCV()
	{
	}

	int _WindowCV::init(void *pKiss)
	{
		CHECK_(this->_UIbase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("bFullScreen", &m_bFullScreen);
		pK->v("vSize", &m_vSize);
		m_waitKey = 1000.0f / m_pT->getTargetFPS();

		if (m_vSize.area() <= 0)
		{
			LOG_E("Window size too small");
			return OK_ERR_INVALID_VALUE;
		}

		m_F.allocate(m_vSize.x, m_vSize.y);

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

		return OK_OK;
	}

	int _WindowCV::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	void _WindowCV::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			updateWindow();

			m_pT->autoFPSto();
		}
	}

	void _WindowCV::updateWindow(void)
	{
		// draw contents
		for (BASE *pB : m_vpB)
		{
			pB->draw((void *)&m_F);
		}
		IF_(m_F.bEmpty());

		// show window
		imshow(this->getName(), *m_F.m());

		int key = waitKey(m_waitKey);
	}
}
