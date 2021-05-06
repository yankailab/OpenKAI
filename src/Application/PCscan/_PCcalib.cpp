/*
 * _PCcalib.cpp
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifdef USE_OPEN3D
#include "_PCcalib.h"

namespace kai
{

	_PCcalib::_PCcalib()
	{
		m_pPS = NULL;
		m_pUIstate = NULL;
		m_modelName = "PCMODEL";

		m_bFullScreen = false;
		m_mouseMode = 0;
		m_rDummyDome = 1000.0;

		m_fProcess.clearAll();
	}

	_PCcalib::~_PCcalib()
	{
	}

	bool _PCcalib::init(void *pKiss)
	{
		IF_F(!this->_PCviewer::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("bFullScreen", &m_bFullScreen);
		pK->v("mouseMode", &m_mouseMode);
		pK->v("rDummyDome", &m_rDummyDome);

		string n;

		n = "";
		pK->v("_PCstream", &n);
		m_pPS = (_PCstream *)(pK->getInst(n));

		return true;
	}

	bool _PCcalib::start(void)
	{
		NULL_F(m_pT);
		IF_F(!m_pT->start(getUpdate, this));

		NULL_F(m_pTui);
		IF_F(!m_pTui->start(getUpdateUI, this));

		return true;
	}

	int _PCcalib::check(void)
	{
		NULL__(m_pPS, -1);

		return this->_PCviewer::check();
	}

	void _PCcalib::update(void)
	{
		m_pT->sleepT(0);

		startScan();

		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			updateScan();

			m_pT->autoFPSto();
		}
	}

	void _PCcalib::startScan(void)
	{
		IF_(check() < 0);

		m_spWin->ShowMsg("Scan", "Initializing");

		m_pPS->clear();

		removeUIpc();
		addDummyDome(m_sPC.next(), m_pPS->nP(), m_rDummyDome);
		updatePC();
		addUIpc(*m_sPC.get());

		resetCamPose();
		updateCamPose();
		
		m_spWin->CloseDialog();
	}

	void _PCcalib::updateScan(void)
	{
		IF_(check() < 0);

		readAllPC();
		PointCloud *pPC = m_sPC.get();
		pPC->normals_.clear();
		int n = pPC->points_.size();
		IF_(n <= 0);
		int nP = m_pPS->nP();

		m_aabb = pPC->GetAxisAlignedBoundingBox();

		PointCloud pc = *pPC;
		if (n < nP)
		{
			addDummyDome(&pc, nP - n, m_rDummyDome);
		}
		else if (n > nP)
		{
			int d = n - nP;
			pc.points_.erase(pc.points_.end() - d, pc.points_.end());
			pc.colors_.erase(pc.colors_.end() - d, pc.colors_.end());
		}

		updateUIpc(pc);
	}

	void _PCcalib::addUIpc(const PointCloud &pc)
	{
		IF_(pc.IsEmpty());

		m_spWin->AddPointCloud(m_modelName,
							   make_shared<t::geometry::PointCloud>(
								   t::geometry::PointCloud::FromLegacyPointCloud(
									   pc,
									   core::Dtype::Float32)));
	}

	void _PCcalib::updateUIpc(const PointCloud &pc)
	{
		IF_(pc.IsEmpty());

		m_spWin->UpdatePointCloud(m_modelName,
								  make_shared<t::geometry::PointCloud>(
									  t::geometry::PointCloud::FromLegacyPointCloud(
										  pc,
										  core::Dtype::Float32)));
	}

	void _PCcalib::removeUIpc(void)
	{
		m_spWin->RemoveGeometry(m_modelName);
	}

	void _PCcalib::updateUI(void)
	{
		auto &app = gui::Application::GetInstance();
		app.Initialize(m_pathRes.c_str());

		m_spWin = std::make_shared<visualizer::PCcalibUI>(*this->getName(), 2000, 1000);
		app.AddWindow(m_spWin);

		m_spWin->SetCbLoadImgs(OnLoadImgs, (void *)this);
		m_spWin->SetCbResetPC(OnResetPC, (void *)this);

		m_pUIstate = m_spWin->getUIState();
		m_pUIstate->m_mouseMode = (visualization::gui::SceneWidget::Controls)m_mouseMode;
		m_spWin->UpdateUIstate();
		m_spWin->SetFullScreen(m_bFullScreen);
		m_aabb = createDefaultAABB();
		camBound(m_aabb);
		updateCamProj();
		updateCamPose();

		m_pT->wakeUp();
		app.Run();
		exit(0);
	}

	void _PCcalib::updateCamProj(void)
	{
		IF_(check() < 0);
		IF_(!m_spWin);

		m_spWin->CamSetProj(m_camProj.m_fov,
							m_camProj.m_vNF.x,
							m_camProj.m_vNF.y,
							m_camProj.m_fovType);
	}

	void _PCcalib::updateCamPose(void)
	{
		IF_(check() < 0);
		IF_(!m_spWin);

		m_spWin->CamSetPose(m_cam.m_vLookAt.v3f(),
							m_cam.m_vEye.v3f(),
							m_cam.m_vUp.v3f());
	}

	void _PCcalib::camBound(const AxisAlignedBoundingBox &aabb)
	{
		IF_(check() < 0);
		IF_(!m_spWin);

		m_spWin->CamAutoBound(aabb, m_vCoR.v3f());
	}

	AxisAlignedBoundingBox _PCcalib::createDefaultAABB(void)
	{
		PointCloud pc;
		pc.points_.push_back(Vector3d(0,0,1));
		pc.points_.push_back(Vector3d(0,0,-1));
		pc.points_.push_back(Vector3d(0,1,0));
		pc.points_.push_back(Vector3d(0,-1,0));
		pc.points_.push_back(Vector3d(1,0,0));
		pc.points_.push_back(Vector3d(-1,0,0));
		return pc.GetAxisAlignedBoundingBox();
	}

	void _PCcalib::OnLoadImgs(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_PCcalib *pV = (_PCcalib *)pPCV;

		if (io::ReadPointCloud((const char *)pD, *pV->m_sPC.next()))
			pV->updatePC();

	}

	void _PCcalib::OnResetPC(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_PCcalib *pV = (_PCcalib *)pPCV;

		if (io::ReadPointCloud((const char *)pD, *pV->m_sPC.next()))
			pV->updatePC();

	}

}
#endif
