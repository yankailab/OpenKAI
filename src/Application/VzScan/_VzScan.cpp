/*
 * _VzScan.cpp
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#include "_VzScan.h"

namespace kai
{
	_VzScan::_VzScan()
	{
		m_pNav = NULL;
		m_pTk = NULL;

		m_pPCprv = NULL;
		m_pPCorig = NULL;
		m_nPwPrv = 0;
		m_nPwOrig = 0;
		m_rVoxel = 0.1;
		m_dHiddenRemove = 100.0;
		m_fProcess.clearAll();
	}

	_VzScan::~_VzScan()
	{
		DEL(m_pTk);
	}

	bool _VzScan::init(void *pKiss)
	{
		IF_F(!this->_GeometryViewer::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		m_pPCprv = m_sPC.get();
		m_pPCorig = m_sPC.next();

		pK->v("rVoxel", &m_rVoxel);
		pK->v("dHiddenRemove", &m_dHiddenRemove);

		string n = "";
		pK->v("_NavBase", &n);
		m_pNav = (_NavBase *)(pK->getInst(n));

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

	bool _VzScan::start(void)
	{
		NULL_F(m_pT);
		IF_F(!m_pT->start(getUpdate, this));

		NULL_F(m_pTk);
		IF_F(!m_pTk->start(getUpdateKinematics, this));

		NULL_F(m_pTui);
		IF_F(!m_pTui->start(getUpdateUI, this));

		return true;
	}

	int _VzScan::check(void)
	{
		NULL__(m_pNav, -1);
		NULL__(m_pWin, -1);
		IF__(m_vpGB.empty(), -1);

		return this->_GeometryViewer::check();
	}

	void _VzScan::update(void)
	{
		m_pT->sleepT(0);

		// init
		m_fProcess.set(pc_ScanReset);

		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			if (m_fProcess.b(pc_ScanReset, true))
			{
				scanReset();
			}

			if (m_fProcess.b(pc_ScanTake, true))
			{
				scanTake();
			}

			//			if (m_fProcess.b(pc_Scanning))
			//			{
			updateScan();
			//			}
			// else
			// {
			// 	updateProcess();
			// }

			// if (m_fProcess.b(pc_ScanStart, true))
			// {
			// 	startScan();
			// }

			// if (m_fProcess.b(pc_ScanStop, true))
			// {
			// 	stopScan();
			// }

			if (m_fProcess.b(pc_CamAuto))
				updateCamAuto();

			m_pT->autoFPSto();
		}
	}

	void _VzScan::scanReset(void)
	{
		IF_(check() < 0);

		m_pWin->ShowMsg("Scan", "Initializing");

//		m_pNav->reset();
		while (!m_pNav->bReady())
			m_pT->sleepT(100000);

		// original point cloud
		m_nPwOrig = 0;
		m_pPCorig->Clear();

		// voxel down point cloud for preview
		m_nPwPrv = 0;
		m_pPCprv->Clear();
		addDummyPoints(m_pPCprv, m_nPresv, m_rDummyDome);

		removeUIpc();
		addUIpc(*m_pPCprv);
		m_fProcess.set(pc_Scanning);

		resetCamPose();
		updateCamPose();

		m_pWin->CloseDialog();
	}

	// void _VzScan::stopScan(void)
	// {
	// 	IF_(check() < 0);

	// 	m_pWin->ShowMsg("Scan", "Processing");

	// 	PointCloud* pP = m_sPC.get();

	// 	if (m_nPdummy > 0)
	// 	{
	// 		pP->points_.erase(pP->points_.end() - m_nPdummy, pP->points_.end());
	// 		pP->colors_.erase(pP->colors_.end() - m_nPdummy, pP->colors_.end());
	// 	}

	// 	removeUIpc();
	// 	addUIpc(*pP);
	// 	m_aabb = pP->GetAxisAlignedBoundingBox();
	// 	camBound(m_aabb);
	// 	updateCamPose();

	// 	m_fProcess.clear(pc_Scanning);
	// 	m_pWin->CloseDialog();
	// }

	void _VzScan::scanTake(void)
	{
		IF_(check() < 0);

		_PCframe *pPsrc = (_PCframe *)m_vpGB[0];
		PointCloud pc;
		pPsrc->getPC(&pc);

		int nPnew = pc.points_.size();
		IF_(nPnew <= 0);

		int i;

		// Add original
		for (i = 0; i < nPnew; i++)
		{
			m_pPCorig->points_[m_nPwOrig] = pc.points_[i];
			m_pPCorig->colors_[m_nPwOrig] = pc.colors_[i];

			m_nPwOrig++;
			if (m_nPwOrig >= m_nPresvNext)
				break;
		}

		// Add voxel down for preview
		PointCloud pcVd = *pc.VoxelDownSample(m_rVoxel);
		int nPvd = pcVd.points_.size();
		for (i = 0; i < nPvd; i++)
		{
			m_pPCprv->points_[m_nPwPrv] = pcVd.points_[i];
			m_pPCprv->colors_[m_nPwPrv] = pcVd.colors_[i];

			m_nPwPrv++;
			if (m_nPwPrv >= m_nPresv)
				break;
		}

		int nDummy = m_nPresv - m_nPwPrv;
		if (nDummy > 0)
		{
			m_pPCprv->points_.erase(m_pPCprv->points_.end() - nDummy,
									m_pPCprv->points_.end());
			m_pPCprv->colors_.erase(m_pPCprv->colors_.end() - nDummy,
									m_pPCprv->colors_.end());
			addDummyPoints(m_pPCprv, nDummy, m_rDummyDome);
		}

		m_aabb = pc.GetAxisAlignedBoundingBox();
		if (m_pUIstate)
			m_pUIstate->m_sMove = m_vDmove.constrain(m_aabb.Volume() * 0.0001);

		updateUIpc(*m_pPCprv);
		_VzScanUI *pW = (_VzScanUI *)m_pWin;
		pW->SetProgressBar((float)m_nPwOrig / (float)m_nPresvNext);
	}

	void _VzScan::updateScan(void)
	{
		IF_(check() < 0);

		_PCframe *pPsrc = (_PCframe *)m_vpGB[0];
		PointCloud pc;
		pPsrc->getPC(&pc);

		int nPnew = pc.points_.size();
		IF_(nPnew <= 0);

		int iPw = m_nPwPrv;
		for (int i = 0; i < nPnew; i++)
		{
			m_pPCprv->points_[iPw] = pc.points_[i];
			m_pPCprv->colors_[iPw] = pc.colors_[i];

			iPw++;
			if (iPw >= m_nPresv)
				break;
		}

		int nDummy = m_nPresv - iPw;
		if (nDummy > 0)
		{
			m_pPCprv->points_.erase(m_pPCprv->points_.end() - nDummy, m_pPCprv->points_.end());
			m_pPCprv->colors_.erase(m_pPCprv->colors_.end() - nDummy, m_pPCprv->colors_.end());
			addDummyPoints(m_pPCprv, nDummy, m_rDummyDome);
		}

		m_aabb = pc.GetAxisAlignedBoundingBox();
		if (m_pUIstate)
			m_pUIstate->m_sMove = m_vDmove.constrain(m_aabb.Volume() * 0.0001);

		updateUIpc(*m_pPCprv);
	}

	void _VzScan::updateCamAuto(void)
	{
		IF_(check() < 0);

		Eigen::Affine3f A;
		A = m_pNav->mT().cast<float>();
		m_cam.m_vEye = A * m_camAuto.m_vEye.v3f();
		m_cam.m_vLookAt = A * m_camAuto.m_vLookAt.v3f();

		updateCamPose();
	}

	void _VzScan::updateProcess(void)
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

	void _VzScan::updateKinematics(void)
	{
		while (m_pTk->bRun())
		{
			m_pTk->autoFPSfrom();

			updateSlam();

			m_pTk->autoFPSto();
		}
	}

	void _VzScan::updateSlam(void)
	{
		IF_(check() < 0);
		//		IF_(!m_fProcess.b(pc_Scanning));

		auto mT = m_pNav->mT();
		for (int i = 0; i < m_vpGB.size(); i++)
		{
			_GeometryBase *pP = m_vpGB[i];
			pP->setTranslation(mT.cast<double>());
		}
	}

	void _VzScan::updateUI(void)
	{
		auto &app = gui::Application::GetInstance();
		app.Initialize(m_pathRes.c_str());

		m_pWin = new _VzScanUI(*this->getName(), 2000, 1000);
		m_pUIstate = m_pWin->getUIState();
		m_pUIstate->m_bSceneCache = m_bSceneCache;
		m_pUIstate->m_mouseMode = (visualization::gui::SceneWidget::Controls)m_mouseMode;
		m_pUIstate->m_wPanel = m_wPanel;
		m_pUIstate->m_sMove = m_vDmove.x;
		m_pUIstate->m_btnPaddingH = m_vBtnPadding.x;
		m_pUIstate->m_btnPaddingV = m_vBtnPadding.y;
		m_pUIstate->m_dirSave = m_dirSave;
		m_pWin->Init();
		_VzScanUI *pW = (_VzScanUI *)m_pWin;
		app.AddWindow(shared_ptr<_VzScanUI>(pW));

		//		pW->SetCbScan(OnScan, (void *)this);
		pW->SetCbScanReset(OnScanReset, (void *)this);
		pW->SetCbScanTake(OnScanTake, (void *)this);
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

		m_pT->wakeUp();
		app.Run();
		exit(0);
	}

	void _VzScan::OnScanReset(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		_VzScan *pV = (_VzScan *)pPCV;

		pV->m_fProcess.set(pc_ScanReset);
	}

	void _VzScan::OnScanTake(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		_VzScan *pV = (_VzScan *)pPCV;

		pV->m_fProcess.set(pc_ScanTake);
	}

	void _VzScan::OnOpenPC(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_VzScan *pV = (_VzScan *)pPCV;

		if (io::ReadPointCloud((const char *)pD, *pV->m_sPC.next()))
			pV->updatePC();

		pV->m_fProcess.set(pc_ResetPC);
	}

	void _VzScan::OnCamSet(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_VzScan *pV = (_VzScan *)pPCV;
		int camMode = *(int *)pD;

		if (camMode == 0) // auto off
		{
			pV->m_fProcess.clear(pc_CamAuto);
		}
		else if (camMode == 1) // auto on
		{
			pV->m_fProcess.set(pc_CamAuto);
		}
		else if (camMode == 3 || pV->m_sPC.get()->points_.empty()) // origin or no point data
		{
			pV->resetCamPose();
			pV->updateCamPose();
		}
		else if (camMode == 4) // all
		{
			pV->camBound(pV->m_aabb);
		}
	}

	void _VzScan::OnHiddenRemove(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		_VzScan *pV = (_VzScan *)pPCV;
		pV->m_fProcess.set(pc_HiddenRemove);
	}

	void _VzScan::OnVoxelDown(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		_VzScan *pV = (_VzScan *)pPCV;
		pV->m_fProcess.set(pc_VoxelDown);
	}

	void _VzScan::OnResetPC(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		_VzScan *pV = (_VzScan *)pPCV;
		pV->m_fProcess.set(pc_ResetPC);
	}

}
