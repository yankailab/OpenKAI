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
		m_vCoR.init(0, 0, 0);
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
		pK->v("vCamNF", &m_camProj.m_vNF);
		pK->v("camFovType", &m_camProj.m_fovType);

		pK->v("vCamLookAt", &m_camDefault.m_vLookAt);
		pK->v("vCamEye", &m_camDefault.m_vEye);
		pK->v("vCamUp", &m_camDefault.m_vUp);
		m_cam = m_camDefault;

		pK->v("vCamAutoLookAt", &m_camAuto.m_vLookAt);
		pK->v("vCamAutoEye", &m_camAuto.m_vEye);
		pK->v("vCamAutoUp", &m_camAuto.m_vUp);

		pK->v("vCoR", &m_vCoR);

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

	void _PCviewer::addDummyDome(PointCloud* pPC, int n, float r, Vector3d vCol)
	{
		NULL_(pPC);

		float nV = floor(sqrt((float)n));
		float nH = ceil(n / nV);

		float dV = OK_PI / nV;
		float dH = (OK_PI * 2.0) / nH;

		int k = 0;
		for(int i=0; i<nH; i++)
		{
			float h = dH * i;
			float sinH = sin(h);
			float cosH = cos(h);

			for(int j=0; j<nV; j++)
			{
				float v = dV * j;
				float sinV = sin(v);
				float cosV = cos(v);

				Vector3d vP
				(
					r * sinV * sinH,
					r * sinV * cosH,
					r * cosV
				);

				pPC->points_.push_back(vP);
				pPC->colors_.push_back(vCol);

				IF_(++k >= n);
			}
		}
	}

	void _PCviewer::resetCamPose(void)
	{
		m_cam = m_camDefault;
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

}
#endif
