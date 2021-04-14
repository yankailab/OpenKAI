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
		m_vWinSize.init(1280, 720);

		m_pTui = NULL;
		m_pathRes = "";
		m_device = "CPU:0";

		m_camProj.init();
		m_vCamCenter.init(0);
		m_vCamEye.init(0, 0, 1);
		m_vCamUp.init(0, 1, 0);
		m_vCamCoR.init(0, 0, 0);
	}

	_PCviewer::~_PCviewer()
	{
	}

	bool _PCviewer::init(void *pKiss)
	{
		IF_F(!this->_PCframe::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vWinSize", &m_vWinSize);
		pK->v("pathRes", &m_pathRes);
		pK->v("device", &m_device);

		pK->v("camFov", &m_camProj.m_fov);
		pK->v("camAspect", &m_camProj.m_aspect);
		pK->v("vCamNF", &m_camProj.m_vNF);
		pK->v("camFovType", &m_camProj.m_fovType);

		pK->v("vCamCenter", &m_vCamCenter);
		pK->v("vCamEye", &m_vCamEye);
		pK->v("vCamUp", &m_vCamUp);
		pK->v("vCamCoR", &m_vCamCoR);

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

		Kiss *pKt = pK->child("threadUI");
		IF_F(pKt->empty());
		m_pTui = new _Thread();
		if (!m_pTui->init(pKt))
		{
			DEL(m_pTui);
			return false;
		}

		return true;
	}

	bool _PCviewer::start(void)
	{
		NULL_F(m_pT);
		IF_F(!m_pT->start(getUpdate, this));
		
		NULL_F(m_pTui);
		IF_F(!m_pTui->start(getUpdateUI, this));

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

		m_spWin->SetGeometry(shared_ptr<PointCloud>(m_sPC.get()));

		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			readAllPC();
			m_spWin->UpdateGeometry(shared_ptr<PointCloud>(m_sPC.get()));

			m_pT->autoFPSto();
		}

	}

	void _PCviewer::readAllPC(void)
	{
		m_nPread = 0;
		for (_PCbase *pPCB : m_vpPCB)
		{
			readPC(pPCB);
		}

		updatePC();
	}

	void _PCviewer::addDummyPC(PointCloud* pPC, int n, double l, int iAxis, Vector3d vCol)
	{
		NULL_(pPC);

		double pFrom = -l*0.5;
		double d = l / n;
		for(int i=0; i<n; i++)
		{
			Vector3d vP(0,0,0);
			vP[iAxis] = pFrom + i * d;
			
			pPC->points_.push_back(vP);
			pPC->colors_.push_back(vCol);
		}
	}

	void _PCviewer::updateUI(void)
	{
		auto &app = gui::Application::GetInstance();
		app.Initialize(m_pathRes.c_str());

		m_spWin = std::make_shared<PCviewerUI>(*this->getName(), 2000, 1000);
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
