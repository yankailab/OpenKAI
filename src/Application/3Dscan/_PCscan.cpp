/*
 * _PCscan.cpp
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#include "_PCscan.h"

namespace kai
{
	_PCscan::_PCscan()
	{
//		m_pPS = NULL;
		m_pNav = NULL;
		m_pTk = NULL;

		m_bSlam = true;
		m_dHiddenRemove = 100.0;
		m_fProcess.clearAll();
	}

	_PCscan::~_PCscan()
	{
		DEL(m_pTk);
	}

	bool _PCscan::init(void *pKiss)
	{
		IF_F(!this->_GeometryViewer::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("bSlam", &m_bSlam);
		pK->v("dHiddenRemove", &m_dHiddenRemove);

		string n = "";
		pK->v("_NavBase", &n);
		m_pNav = (_NavBase *)(pK->findModule(n));

		// n = "";
		// pK->v("_PCstream", &n);
		// m_pPS = (_PCstream *)(pK->findModule(n));

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

	bool _PCscan::start(void)
	{
		NULL_F(m_pT);
		IF_F(!m_pT->start(getUpdate, this));

		NULL_F(m_pTk);
		IF_F(!m_pTk->start(getUpdateKinematics, this));

		NULL_F(m_pTui);
		IF_F(!m_pTui->start(getUpdateUI, this));

		return true;
	}

	int _PCscan::check(void)
	{
//		NULL__(m_pPS, -1);
		NULL__(m_pNav, -1);
		NULL__(m_pWin, -1);
		IF__(m_vpGB.empty(),-1);

		return this->_GeometryViewer::check();
	}

	void _PCscan::update(void)
	{
		m_pT->sleepT(0);

		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			if (m_fProcess.b(pc_ScanStart, true))
			{
				startScan();
			}

			if (m_fProcess.b(pc_ScanStop, true))
			{
				stopScan();
			}

			if (m_fProcess.b(pc_Scanning))
			{
				updateScan();
			}
			else
			{
				updateProcess();
			}

			if (m_fProcess.b(pc_CamAuto))
				updateCamAuto();

			m_pT->autoFPSto();
		}
	}

	void _PCscan::startScan(void)
	{
		IF_(check() < 0);

		m_pWin->ShowMsg("Scan", "Initializing");

//		m_pNav->reset();
		while(!m_pNav->bReady())
			m_pT->sleepT(100000);

		_PCstream* pPS = (_PCstream*)m_vpGB[0];
		pPS->clear();
		pPS->startStream();

		removeUIpc();
		addDummyPoints(m_sPC.next(), pPS->nP(), m_rDummyDome);
		updatePC();
		addUIpc(*m_sPC.get());
		m_fProcess.set(pc_Scanning);

		resetCamPose();
		updateCamPose();
		
		m_pWin->CloseDialog();
	}

	void _PCscan::stopScan(void)
	{
		IF_(check() < 0);

		m_pWin->ShowMsg("Scan", "Processing");
		_PCstream* pPS = (_PCstream*)m_vpGB[0];
		pPS->stopStream();

		removeUIpc();
		addUIpc(*m_sPC.get());
		m_aabb = m_sPC.get()->GetAxisAlignedBoundingBox();
		camBound(m_aabb);
		updateCamPose();

		m_fProcess.clear(pc_Scanning);
		m_pWin->CloseDialog();
	}

	void _PCscan::updateScan(void)
	{
		IF_(check() < 0);

		updateAllGeometries();
		updatePC();
		PointCloud *pPC = m_sPC.get();
		pPC->normals_.clear();
		int n = pPC->points_.size();
		IF_(n <= 0);
		
		_PCstream* pPS = (_PCstream*)m_vpGB[0];
		int nP = pPS->nP();

		m_aabb = pPC->GetAxisAlignedBoundingBox();
		if(m_pUIstate)
			m_pUIstate->m_sMove = m_vDmove.constrain(m_aabb.Volume() * 0.0001);

		PointCloud pc = *pPC;
		if (n < nP)
		{
			addDummyPoints(&pc, nP - n, m_rDummyDome);
		}
		else if (n > nP)
		{
			int d = n - nP;
			pc.points_.erase(pc.points_.end() - d, pc.points_.end());
			pc.colors_.erase(pc.colors_.end() - d, pc.colors_.end());
		}

		updateUIpc(pc);
		PCscanUI* pW = (PCscanUI*)m_pWin;
		pW->SetProgressBar((float)pPS->iP() / (float)pPS->nP());
	}

	void _PCscan::updateCamAuto(void)
	{
		IF_(check() < 0);

		Eigen::Affine3f A;
		A = m_pNav->mT().cast<float>();
		m_cam.m_vEye = A * m_camAuto.m_vEye.v3f();
		m_cam.m_vLookAt = A * m_camAuto.m_vLookAt.v3f();

		updateCamPose();
	}

	void _PCscan::updateProcess(void)
	{
		IF_(check() < 0);

		if (m_fProcess.b(pc_ResetPC, true))
		{
			removeUIpc();
			addUIpc(*m_sPC.get());
			m_aabb = m_sPC.get()->GetAxisAlignedBoundingBox();
			camBound(m_aabb);
		}

		if (m_fProcess.b(pc_VoxelDown, true) && m_pUIstate)
		{
			m_pWin->ShowMsg("Voxel Down Sampling", "Processing");

			removeUIpc();
			PointCloud pc;
			float s = m_pUIstate->m_sVoxel;
			if (s > 0.0)
				pc = *m_sPC.get()->VoxelDownSample(s);
			else
				pc = *m_sPC.get();

			m_aabb = pc.GetAxisAlignedBoundingBox();
			addUIpc(pc);
			m_pWin->CloseDialog();
		}

		if (m_fProcess.b(pc_HiddenRemove, true) && m_pUIstate)
		{
			m_pWin->ShowMsg("Hidden Point Removal", "Processing");

			removeUIpc();

			PointCloud pc = *m_sPC.get();
			auto pcR = pc.HiddenPointRemoval(m_pUIstate->m_vCamPos.cast<double>(), m_dHiddenRemove);
			pc = *pc.SelectByIndex(std::get<1>(pcR));

			addUIpc(pc);
			m_pWin->CloseDialog();
		}
	}

	void _PCscan::updateKinematics(void)
	{
		while (m_pTk->bAlive())
		{
			m_pTk->autoFPSfrom();

			updateSlam();

			m_pTk->autoFPSto();
		}
	}

	void _PCscan::updateSlam(void)
	{
		IF_(check() < 0);
		IF_(!m_fProcess.b(pc_Scanning));
		IF_(!m_bSlam);

		auto mT = m_pNav->mT();
		for (int i = 0; i < m_vpGB.size(); i++)
		{
			_GeometryBase *pP = m_vpGB[i];
			pP->setTranslation(mT.cast<double>());
		}
	}

	void _PCscan::updateUI(void)
	{
		auto &app = gui::Application::GetInstance();
		app.Initialize(m_pathRes.c_str());

		m_pWin = new PCscanUI(this->getName(), 2000, 1000);
		m_pUIstate = m_pWin->getUIState();
		m_pUIstate->m_bSceneCache = m_bSceneCache;
		m_pUIstate->m_mouseMode = (visualization::gui::SceneWidget::Controls)m_mouseMode;
		m_pUIstate->m_wPanel = m_wPanel;
		m_pUIstate->m_sMove = m_vDmove.x;
		m_pUIstate->m_btnPaddingH = m_vBtnPadding.x;
		m_pUIstate->m_btnPaddingV = m_vBtnPadding.y;
		m_pUIstate->m_dirSave = m_dirSave;
		m_pWin->Init();
		PCscanUI* pW = (PCscanUI*)m_pWin;
		app.AddWindow(shared_ptr<PCscanUI>(pW));

		pW->SetCbScan(OnScan, (void *)this);
		pW->SetCbOpenPC(OnOpenPC, (void *)this);
		pW->SetCbCamSet(OnCamSet, (void *)this);
		pW->SetCbHiddenRemove(OnHiddenRemove, (void *)this);
		pW->SetCbResetPC(OnResetPC, (void *)this);
		pW->SetCbVoxelDown(OnVoxelDown, (void *)this);

		m_pWin->UpdateUIstate();
//		m_pWin->SetFullScreen(m_bFullScreen);
		m_aabb = createDefaultAABB();
		camBound(m_aabb);
		updateCamProj();
		updateCamPose();

		m_pT->run();
		app.Run();
		exit(0);
	}

	void _PCscan::OnScan(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_PCscan *pV = (_PCscan *)pPCV;

		if (*((bool *)pD))
			pV->m_fProcess.set(pc_ScanStart);
		else
			pV->m_fProcess.set(pc_ScanStop);
	}

	void _PCscan::OnOpenPC(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_PCscan *pV = (_PCscan *)pPCV;

		if (io::ReadPointCloud((const char *)pD, *pV->m_sPC.next()))
			pV->updatePC();

		pV->m_fProcess.set(pc_ResetPC);
	}

	void _PCscan::OnCamSet(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_PCscan *pV = (_PCscan *)pPCV;
		int camMode = *(int *)pD;

		if(camMode == 0)	//auto off
		{
			pV->m_fProcess.clear(pc_CamAuto);
		}
		else if(camMode == 1)	//auto on
		{
			pV->m_fProcess.set(pc_CamAuto);
		}
		else if(camMode == 3 || pV->m_sPC.get()->points_.empty())	//origin or no point data
		{
			pV->resetCamPose();
			pV->updateCamPose();
		}
		else if(camMode == 4)	//all
		{
			pV->camBound(pV->m_aabb);
		}
	}

	void _PCscan::OnHiddenRemove(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		_PCscan *pV = (_PCscan *)pPCV;
		pV->m_fProcess.set(pc_HiddenRemove);
	}

	void _PCscan::OnVoxelDown(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		_PCscan *pV = (_PCscan *)pPCV;
		pV->m_fProcess.set(pc_VoxelDown);
	}

	void _PCscan::OnResetPC(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		_PCscan *pV = (_PCscan *)pPCV;
		pV->m_fProcess.set(pc_ResetPC);
	}

}
