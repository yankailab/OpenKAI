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

		m_nP = 0;
		m_nPmax = INT_MAX;
		m_pPCprv = NULL;
		m_iPprv = 0;
		m_rVoxel = 0.1;
		m_fProcess.clearAll();

		m_fNameSavePC = "";
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

		pK->v("rVoxel", &m_rVoxel);
		pK->v("nPmax", &m_nPmax);

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
				scanReset();

			if (m_fProcess.b(pc_ScanTake, true))
				scanTake();

			if (m_fProcess.b(pc_SavePC, true))
				savePC();

			updateScan();

			if (m_fProcess.b(pc_CamAuto))
				updateCamAuto();

			if (m_fProcess.b(pc_CamCtrl, true))
				updateCamCtrl();

			m_pT->autoFPSto();
		}
	}

	void _VzScan::scanReset(void)
	{
		IF_(check() < 0);

		m_pWin->ShowMsg("Scan", "Initializing");

		//		m_pNav->reset();

		m_nP = 0;
		for (int i = 0; i < m_vPC.size(); i++)
		{
			PointCloud *pP = &m_vPC[i];
			pP->Clear();
		}
		m_vPC.clear();

		// voxel down point cloud for preview
		m_iPprv = 0;
		m_pPCprv->Clear();
		addDummyPoints(m_pPCprv, m_nPresv, m_rDummyDome);

		removeUIpc();
		addUIpc(*m_pPCprv);
		m_fProcess.set(pc_Scanning);

		resetCamPose();
		updateCamPose();

		m_pWin->CloseDialog();
	}

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
		m_vPC.push_back(pc);
		m_nP += pc.points_.size();
		// TODO: check point number

		// for (i = 0; i < nPnew; i++)
		// {
		// 	m_pPCorig->points_.push_back(pc.points_[i]);
		// 	m_pPCorig->colors_.push_back(pc.colors_[i]);
		// 	m_nPwOrig++;
		// 	if (m_nPwOrig >= m_nPresvNext)
		// 		break;
		// }

		// Add voxel down for preview
		PointCloud pcVd = *pc.VoxelDownSample(m_rVoxel);
		int nPvd = pcVd.points_.size();
		for (i = 0; i < nPvd; i++)
		{
			m_pPCprv->points_[m_iPprv] = pcVd.points_[i];
			m_pPCprv->colors_[m_iPprv] = pcVd.colors_[i];
			m_iPprv++;
			if (m_iPprv >= m_nPresv)
				break;
		}

		int nDummy = m_nPresv - m_iPprv;
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
		float rPorig = (float)m_nP / (float)m_nPmax;
		float rPprv = (float)m_iPprv / (float)m_nPresv;
		pW->SetProgressBar(max(rPprv, rPorig));
	}

	void _VzScan::updateScan(void)
	{
		IF_(check() < 0);

		_PCframe *pPsrc = (_PCframe *)m_vpGB[0];
		PointCloud pc;
		pPsrc->getPC(&pc);

		int nPnew = pc.points_.size();
		IF_(nPnew <= 0);

		int iPw = m_iPprv;
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
	}

	void _VzScan::savePC(void)
	{
		IF_(check() < 0);
		if (m_nP <= 0)
		{
			m_pWin->ShowMsg("FILE", "Model is empty", true);
			return;
		}

		m_pWin->ShowMsg("FILE", "Saving .PLY file");

		io::WritePointCloudOption par;
		par.write_ascii = io::WritePointCloudOption::IsAscii::Binary;
		par.compressed = io::WritePointCloudOption::Compressed::Uncompressed;

		PointCloud pcMerge;
		int nSave = 0;
		for (int i = 0; i < m_vPC.size(); i++)
		{
			PointCloud* pP = &m_vPC[i];
			nSave += (io::WritePointCloudToPLY(m_fNameSavePC + i2str(i) + ".ply",
											 *pP,
											 par))?1:0;

			pcMerge += *pP;
		}

		nSave += (io::WritePointCloudToPLY(m_fNameSavePC + "_merged.ply",
										 pcMerge,
										 par))?1:0;

		m_pWin->CloseDialog();
		string msg;
		if (nSave > m_vPC.size())
		{
			msg = "Saved to: " + m_fNameSavePC;
			m_pWin->ShowMsg("FILE", msg.c_str(), true);
		}
		else
		{
			msg = "Failed to save: " + m_fNameSavePC;
			m_pWin->ShowMsg("FILE", msg.c_str(), true);
		}
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

	void _VzScan::updateCamCtrl(void)
	{
		IF_(check() < 0);

		_VzensePC *pVz = (_VzensePC *)m_vpGB[0];
		NULL_(pVz);

		pVz->setCamCtrl(m_camCtrl);
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
		IF_(!m_pNav->bReady())
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

		pW->SetCbScanReset(OnScanReset, (void *)this);
		pW->SetCbScanTake(OnScanTake, (void *)this);
		pW->SetCbSavePC(OnSavePC, (void *)this);
		pW->SetCbCamSet(OnCamSet, (void *)this);
		pW->SetCbCamCtrl(OnCamCtrl, (void *)this);

		m_pWin->UpdateUIstate();
		m_pWin->SetFullScreen(m_bFullScreen);
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

	void _VzScan::OnSavePC(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		_VzScan *pV = (_VzScan *)pPCV;

		pV->m_fNameSavePC = *(string *)pD;
		pV->m_fProcess.set(pc_SavePC);
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

	void _VzScan::OnCamCtrl(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_VzScan *pV = (_VzScan *)pPCV;

		pV->m_camCtrl = *(VzCtrl *)pD;
		pV->m_fProcess.set(pc_CamCtrl);
	}
}
