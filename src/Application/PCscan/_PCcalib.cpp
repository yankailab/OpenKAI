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
		m_pTk = NULL;
		m_pSB = NULL;
		m_pUIstate = NULL;
		m_modelName = "PCMODEL";

		m_bFullScreen = false;
		m_bSceneCache = false;
		m_wPanel = 15;
		m_mouseMode = 0;
		m_vDmove.init(0.5, 5.0);
		m_rDummyDome = 1000.0;
		m_dHiddenRemove = 100.0;

		m_fProcess.clearAll();
	}

	_PCcalib::~_PCcalib()
	{
		DEL(m_pTk);
	}

	bool _PCcalib::init(void *pKiss)
	{
		IF_F(!this->_PCviewer::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("bFullScreen", &m_bFullScreen);
		pK->v("bSceneCache", &m_bSceneCache);
		pK->v("wPanel", &m_wPanel);
		pK->v("mouseMode", &m_mouseMode);
		pK->v("vDmove", &m_vDmove);
		pK->v("rDummyDome", &m_rDummyDome);
		pK->v("dHiddenRemove", &m_dHiddenRemove);

		string n = "";
		pK->v("_SlamBase", &n);
		m_pSB = (_SlamBase *)(pK->getInst(n));

		n = "";
		pK->v("_PCstream", &n);
		m_pPS = (_PCstream *)(pK->getInst(n));

		Kiss *pKk = pK->child("threadK");
		IF_F(pKk->empty());
		m_pTk = new _Thread();
		if (!m_pTk->init(pKk))
		{
			DEL(m_pTk);
			return false;
		}

		return true;
	}

	bool _PCcalib::start(void)
	{
		NULL_F(m_pT);
		IF_F(!m_pT->start(getUpdate, this));

		NULL_F(m_pTk);
		IF_F(!m_pTk->start(getUpdateKinematics, this));

		NULL_F(m_pTui);
		IF_F(!m_pTui->start(getUpdateUI, this));

		return true;
	}

	int _PCcalib::check(void)
	{
		NULL__(m_pPS, -1);
		NULL__(m_pSB, -1);

		return this->_PCviewer::check();
	}

	void _PCcalib::update(void)
	{
		m_pT->sleepT(0);

		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			// if (m_fProcess.b(pcfScanStart))
			// {
			// 	startScan();
			// }

			m_pT->autoFPSto();
		}
	}

	void _PCcalib::startScan(void)
	{
		IF_(check() < 0);

		m_spWin->ShowMsg("Scan", "Initializing");

		m_pSB->reset();
		while(!m_pSB->bReady())
			m_pT->sleepT(100000);

		m_pPS->clear();

		removeUIpc();
		addDummyDome(m_sPC.next(), m_pPS->nP(), m_rDummyDome);
		updatePC();
		addUIpc(*m_sPC.get());
//		m_fProcess.set(pcfScanning);

		resetCamPose();
		updateCamPose();
		
		m_spWin->CloseDialog();
	}

	void _PCcalib::stopScan(void)
	{
		IF_(check() < 0);

		m_spWin->ShowMsg("Scan", "Processing");

		removeUIpc();
		addUIpc(*m_sPC.get());
		m_aabb = m_sPC.get()->GetAxisAlignedBoundingBox();
		camBound(m_aabb);
		updateCamPose();

//		m_fProcess.clear(pcfScanning);
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
		if(m_pUIstate)
			m_pUIstate->m_sMove = m_vDmove.constrain(m_aabb.Volume() * 0.0001);

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

	void _PCcalib::updateCamAuto(void)
	{
		IF_(check() < 0);

		Eigen::Affine3f A;
		A = m_pSB->mT().cast<float>();
		m_cam.m_vEye = A * m_camAuto.m_vEye.v3f();
		m_cam.m_vLookAt = A * m_camAuto.m_vLookAt.v3f();

		updateCamPose();
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

	void _PCcalib::updateKinematics(void)
	{
		while (m_pTk->bRun())
		{
			m_pTk->autoFPSfrom();

			updateSlam();

			m_pTk->autoFPSto();
		}
	}

	void _PCcalib::updateSlam(void)
	{
		IF_(check() < 0);
//		IF_(!m_fProcess.b(pcfScanning, false));

		auto mT = m_pSB->mT();
		for (int i = 0; i < m_vpPCB.size(); i++)
		{
			_PCbase *pP = m_vpPCB[i];
			pP->setTranslation(mT);
		}
	}

	void _PCcalib::updateUI(void)
	{
		auto &app = gui::Application::GetInstance();
		app.Initialize(m_pathRes.c_str());

		m_spWin = std::make_shared<visualizer::PCcalibUI>(*this->getName(), 2000, 1000);
		app.AddWindow(m_spWin);

//		m_spWin->SetCbBtnScan(OnBtnScan, (void *)this);

		m_pUIstate = m_spWin->getUIState();
		m_pUIstate->m_bSceneCache = m_bSceneCache;
		m_pUIstate->m_mouseMode = (visualization::gui::SceneWidget::Controls)m_mouseMode;
		m_pUIstate->m_wPanel = m_wPanel;
		m_pUIstate->m_sMove = m_vDmove.x;
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

	void _PCcalib::OnBtnLoadImgs(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_PCcalib *pV = (_PCcalib *)pPCV;

		if (io::ReadPointCloud((const char *)pD, *pV->m_sPC.next()))
			pV->updatePC();

	}

}
#endif
