/*
 * _PCviewer.cpp
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifdef USE_OPEN3D
#include "_PCviewer.h"

namespace kai
{

	_PCviewer::_PCviewer()
	{
		m_fov = 0.0;
		m_vWinSize.init(1280, 720);

		m_spPC = shared_ptr<PointCloud>(new PointCloud);
		m_pTgui = NULL;
		m_pathRes = "";
		m_device = "CPU:0";
	}

	_PCviewer::~_PCviewer()
	{
	}

	bool _PCviewer::init(void *pKiss)
	{
		IF_F(!this->_PCframe::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vWinSize", &m_vWinSize);
		pK->v("fov", &m_fov);
		pK->v("pathRes", &m_pathRes);
		pK->v("device", &m_device);

		utility::SetVerbosityLevel(utility::VerbosityLevel::Error);

		string n;
		vector<string> vPCB;
		pK->a("vPCbase", &vPCB);

		for (string p : vPCB)
		{
			_PCbase *pPCB = (_PCbase *)(pK->getInst(p));
			IF_CONT(!pPCB);

			m_vpPCB.push_back(pPCB);
		}

		Kiss *pKt = pK->child("thread");
		IF_F(pKt->empty());
		m_pTgui = new _Thread();
		if (!m_pTgui->init(pKt))
		{
			DEL(m_pTgui);
			return false;
		}

		return true;
	}

	bool _PCviewer::start(void)
	{
		NULL_F(m_pT);
		IF_F(!m_pT->start(getUpdate, this));
		
		NULL_F(m_pTgui);
		IF_F(!m_pTgui->start(getUpdateGUI, this));

		return true;
	}

	int _PCviewer::check(void)
	{
		return this->_PCbase::check();
	}

	void _PCviewer::update(void)
	{
		//wait for the UI thread to get window ready
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

	void _PCviewer::readAllPC(void)
	{
		m_nPread = 0;
		for (_PCbase *pPCB : m_vpPCB)
		{
			readPC(pPCB);
			m_nPread += pPCB->nPread();
		}

		updatePC();
		*m_spPC = *m_sPC.prev();
	}

	void _PCviewer::updateGUI(void)
	{
		auto &app = gui::Application::GetInstance();
		app.Initialize(m_pathRes.c_str());

		m_spWin = std::make_shared<WindowO3D>(*this->getName(), 2000, 1000);
		gui::Application::GetInstance().AddWindow(m_spWin);

		m_pT->wakeUp();
		app.Run();
	}

	void _PCviewer::draw(void)
	{
		this->_PCframe::draw();
	}

}
#endif
