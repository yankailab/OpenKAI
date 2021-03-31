/*
 * _PCscanView.cpp
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifdef USE_OPEN3D
#include "_PCscanView.h"

namespace kai
{

	_PCscanView::_PCscanView()
	{
	}

	_PCscanView::~_PCscanView()
	{
	}

	bool _PCscanView::init(void *pKiss)
	{
		IF_F(!this->_PCviewer::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		return true;
	}

	bool _PCscanView::start(void)
	{
		NULL_F(m_pT);
		IF_F(!m_pT->start(getUpdate, this));
		
		NULL_F(m_pTgui);
		IF_F(!m_pTgui->start(getUpdateGUI, this));

		return true;
	}

	int _PCscanView::check(void)
	{
		return this->_PCviewer::check();
	}

	void _PCscanView::update(void)
	{
		m_pT->sleepT(0);

		while(m_nPread <= 0)
			readAllPC();

		m_spWin->SetGeometry(m_spPC, false);

		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			readAllPC();
			m_spWin->UpdateGeometry(m_spPC);

			m_pT->autoFPSto();
		}
	}

	void _PCscanView::updateGUI(void)
	{
		auto &app = gui::Application::GetInstance();
		app.Initialize(m_pathRes.c_str());

		m_spWin = std::make_shared<WindowPCscan>(*this->getName(), 2000, 1000);
		gui::Application::GetInstance().AddWindow(m_spWin);

		m_pT->wakeUp();
		app.Run();
	}

}
#endif
