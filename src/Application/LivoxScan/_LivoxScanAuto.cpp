/*
 * _LivoxScanAuto.cpp
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#include "_LivoxScanAuto.h"

namespace kai
{
	_LivoxScanAuto::_LivoxScanAuto()
	{
		m_pNav = NULL;
		m_pTk = NULL;

		m_nP = 0;
		m_nPmax = INT_MAX;
		m_pPCprv = NULL;
		m_iPprv = 0;
		m_rVoxel = 0.1;
		m_fProcess.clearAll();
		m_baseDir = "";
		m_dir = "";
		m_tWaitSec = 2;

		m_pAct = NULL;
	}

	_LivoxScanAuto::~_LivoxScanAuto()
	{
		DEL(m_pTk);
	}

	bool _LivoxScanAuto::init(void *pKiss)
	{
		IF_F(!this->_GeometryViewer::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		m_pPCprv = m_sPC.get();

		pK->v("rVoxel", &m_rVoxel);
		pK->v("nPmax", &m_nPmax);
		pK->v("baseDir", &m_baseDir);
		pK->v("tWaitSec", &m_tWaitSec);

		Kiss *pKa = pK->child("actuatorH");
		IF_F(pKa->empty());
		pKa->v("vTarget", &m_actH.m_vTarget);
		pKa->v("dV", &m_actH.m_dV);
		pKa->v("iAxis", &m_actH.m_iAxis);

		pKa = pK->child("actuatorV");
		IF_F(pKa->empty());
		pKa->v("vTarget", &m_actV.m_vTarget);
		pKa->v("dV", &m_actV.m_dV);
		pKa->v("iAxis", &m_actV.m_iAxis);

		pKa = pK->child("scanSet");
		if (!pKa->empty())
		{
			pKa->v("vRangeH", &m_scanSet.m_vSvRangeH);
			pKa->v("vRangeV", &m_scanSet.m_vSvRangeV);
			pKa->v("nH", &m_scanSet.m_nH);
			pKa->v("nV", &m_scanSet.m_nV);
		}

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

	bool _LivoxScanAuto::link(void)
	{
		IF_F(!this->_GeometryViewer::link());
		IF_F(!m_pTk->link());

		Kiss *pK = (Kiss *)m_pKiss;

		string n = "";
		F_ERROR_F(pK->v("_NavBase", &n));
		m_pNav = (_NavBase *)(pK->getInst(n));
		NULL_Fl(m_pNav, n + ": not found");

		n = "";
		F_ERROR_F(pK->v("_ActuatorBase", &n));
		m_pAct = (_ActuatorBase *)(pK->getInst(n));
		NULL_Fl(m_pAct, n + ": not found");

		return true;
	}

	bool _LivoxScanAuto::start(void)
	{
		NULL_F(m_pT);
		IF_F(!m_pT->start(getUpdate, this));

		NULL_F(m_pTk);
		IF_F(!m_pTk->start(getUpdateKinematics, this));

		NULL_F(m_pTui);
		IF_F(!m_pTui->start(getUpdateUI, this));

		return true;
	}

	int _LivoxScanAuto::check(void)
	{
		NULL__(m_pNav, -1);
		NULL__(m_pWin, -1);
		NULL__(m_pAct, -1);
		IF__(m_vpGB.empty(), -1);

		return this->_GeometryViewer::check();
	}

	void _LivoxScanAuto::update(void)
	{
		m_pT->sleepT(0);

		// init
		m_fProcess.set(pc_ScanReset);

		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			if (m_fProcess.b(pc_ScanReset, true))
				scanReset();

			if (m_fProcess.b(pc_ScanSet, true))
				scanSet();

			if (m_fProcess.b(pc_ScanStart, true))
				scanStart();

			if (m_fProcess.b(pc_ScanStop, true))
				scanStop();

			scanUpdate();

			updatePreview();

			if (m_fProcess.b(pc_SavePC, true))
				savePC();

			if (m_fProcess.b(pc_CamAuto))
				updateCamAuto();

			if (m_fProcess.b(pc_CamCtrl, true))
				updateCamCtrl();

			m_pT->autoFPSto();
		}
	}

	void _LivoxScanAuto::scanReset(void)
	{
		IF_(check() < 0);

		m_pWin->ShowMsg("Scan", "Initializing");

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

	void _LivoxScanAuto::scanSet(void)
	{
		IF_(check() < 0);
		IF_(m_bScanning);

		_LivoxScanAutoUI *pW = (_LivoxScanAutoUI *)m_pWin;
		m_scanSet = pW->GetScanSet();

		switch (m_scanSet.m_lastSet)
		{
		case lvc_HL:
			m_actH.setTarget(m_scanSet.m_vSvRangeH.x);
			break;
		case lvc_HR:
			m_actH.setTarget(m_scanSet.m_vSvRangeH.y);
			break;
		case lvc_VT:
			m_actV.setTarget(m_scanSet.m_vSvRangeV.x);
			break;
		case lvc_VB:
			m_actV.setTarget(m_scanSet.m_vSvRangeV.y);
			break;
		case lvc_Rst:
			m_actH.setTarget(m_scanSet.m_vSvRangeH.mid());
			m_actV.setTarget(m_scanSet.m_vSvRangeV.mid());
			break;
		}
	}

	void _LivoxScanAuto::scanStart(void)
	{
		IF_(check() < 0);
		_LivoxScanAutoUI *pW = (_LivoxScanAutoUI *)m_pWin;
		m_scanSet = pW->GetScanSet();

		m_nTake = m_scanSet.m_nH * m_scanSet.m_nV;
		m_npH = m_scanSet.m_vSvRangeH.x;
		m_ndH = m_scanSet.m_vSvRangeH.d() / m_scanSet.m_nH;
		m_npV = m_scanSet.m_vSvRangeV.x;
		m_ndV = m_scanSet.m_vSvRangeV.d() / m_scanSet.m_nV;

		m_actH.setTarget(m_npH);
		m_actV.setTarget(m_npV);
		m_bScanning = true;
	}

	void _LivoxScanAuto::scanUpdate(void)
	{
		IF_(check() < 0);

		_LivoxScanAutoUI *pW = (_LivoxScanAutoUI *)m_pWin;
		m_scanSet = pW->GetScanSet();

		IF_(!m_actH.bComplete());
		IF_(!m_actV.bComplete());
		IF_(!m_bScanning);

		// Scanning
		sleep(m_tWaitSec);
		scanTake();

		m_npV += m_ndV;
		if (m_npV > m_scanSet.m_vSvRangeV.y)
		{
			m_npV = m_scanSet.m_vSvRangeV.x;

			m_npH += m_ndH;
			if (m_npH > m_scanSet.m_vSvRangeH.y)
			{
				m_bScanning = false;
				m_actH.setTarget(m_scanSet.m_vSvRangeH.mid());
				m_actV.setTarget(m_scanSet.m_vSvRangeV.mid());
				// pW->SetIsScanning(false);
				// pW->UpdateUIstate();
				return;
			}
		}

		m_actV.setTarget(m_npV);
		m_actH.setTarget(m_npH);
	}

	void _LivoxScanAuto::scanTake(void)
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
		_LivoxScanAutoUI *pW = (_LivoxScanAutoUI *)m_pWin;
		float rPorig = (float)m_nP / (float)m_nPmax;
		float rPprv = (float)m_iPprv / (float)m_nPresv;
		pW->SetProgressBar(max(rPprv, rPorig));
	}

	void _LivoxScanAuto::scanStop(void)
	{
		IF_(check() < 0);

		m_bScanning = false;
		m_actH.setTarget(m_scanSet.m_vSvRangeH.mid());
		m_actV.setTarget(m_scanSet.m_vSvRangeV.mid());
	}

	void _LivoxScanAuto::updatePreview(void)
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

	void _LivoxScanAuto::savePC(void)
	{
		IF_(check() < 0);
		if (m_nP <= 0)
		{
			m_pWin->ShowMsg("FILE", "Model is empty", true);
			return;
		}

		m_pWin->ShowMsg("FILE", "Saving .PLY file");

		// Make new folder
		m_dir = m_baseDir + tFormat() + "/";
		string cmd = "mkdir " + m_dir;
		system(cmd.c_str());

		io::WritePointCloudOption par;
		par.write_ascii = io::WritePointCloudOption::IsAscii::Binary;
		par.compressed = io::WritePointCloudOption::Compressed::Uncompressed;

		PointCloud pcMerge;
		int nSave = 0;
		for (int i = 0; i < m_vPC.size(); i++)
		{
			PointCloud *pP = &m_vPC[i];
			nSave += (io::WritePointCloudToPLY(m_dir + i2str(i) + ".ply",
											   *pP,
											   par)) ? 1 : 0;

			pcMerge += *pP;
		}

		nSave += (io::WritePointCloudToPLY(m_dir + "_merged.ply",
										   pcMerge,
										   par)) ? 1 : 0;

		m_pWin->CloseDialog();
		string msg;
		if (nSave > m_vPC.size())
		{
			msg = "Saved to: " + m_dir;
			m_pWin->ShowMsg("FILE", msg.c_str(), true);
		}
		else
		{
			msg = "Failed to save: " + m_dir;
			m_pWin->ShowMsg("FILE", msg.c_str(), true);
		}
	}

	void _LivoxScanAuto::updateCamAuto(void)
	{
		IF_(check() < 0);

		Eigen::Affine3f A;
		A = m_pNav->mT().cast<float>();
		m_cam.m_vEye = A * m_camAuto.m_vEye.v3f();
		m_cam.m_vLookAt = A * m_camAuto.m_vLookAt.v3f();

		updateCamPose();
	}

	void _LivoxScanAuto::updateCamCtrl(void)
	{
		IF_(check() < 0);

		_VzensePC *pVz = (_VzensePC *)m_vpGB[0];
		NULL_(pVz);

		pVz->setCamCtrl(m_camCtrl);
	}

	void _LivoxScanAuto::updateKinematics(void)
	{
		while (m_pTk->bRun())
		{
			m_pTk->autoFPSfrom();

			updateSlam();

			m_pAct->setPtarget(m_actH.m_iAxis, m_actH.update(), true);
			m_pAct->setPtarget(m_actV.m_iAxis, m_actV.update(), true);

			m_pTk->autoFPSto();
		}
	}

	void _LivoxScanAuto::updateSlam(void)
	{
		IF_(check() < 0);
		IF_(!m_pNav->bReady())

		auto mT = m_pNav->mT();
		for (int i = 0; i < m_vpGB.size(); i++)
		{
			_GeometryBase *pP = m_vpGB[i];
			pP->setTranslation(mT.cast<double>());
		}
	}

	void _LivoxScanAuto::updateUI(void)
	{
		auto &app = gui::Application::GetInstance();
		app.Initialize(m_pathRes.c_str());

		m_pWin = new _LivoxScanAutoUI(*this->getName(), 2000, 1000);
		m_pUIstate = m_pWin->getUIState();
		m_pUIstate->m_bSceneCache = m_bSceneCache;
		m_pUIstate->m_mouseMode = (visualization::gui::SceneWidget::Controls)m_mouseMode;
		m_pUIstate->m_wPanel = m_wPanel;
		m_pUIstate->m_sMove = m_vDmove.x;
		m_pUIstate->m_btnPaddingH = m_vBtnPadding.x;
		m_pUIstate->m_btnPaddingV = m_vBtnPadding.y;
		m_pUIstate->m_dirSave = m_dirSave;
		m_pWin->Init();

		_LivoxScanAutoUI *pW = (_LivoxScanAutoUI *)m_pWin;
		app.AddWindow(shared_ptr<_LivoxScanAutoUI>(pW));

		pW->SetCbScanReset(OnScanReset, (void *)this);
		pW->SetCbScanSet(OnScanSet, (void *)this);
		pW->SetCbScanStart(OnScanStart, (void *)this);
		pW->SetCbScanStop(OnScanStop, (void *)this);

		pW->SetCbSavePC(OnSavePC, (void *)this);
		pW->SetCbCamSet(OnCamSet, (void *)this);
		pW->SetCbCamCtrl(OnCamCtrl, (void *)this);

		pW->SetScanSet(m_scanSet);

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

	void _LivoxScanAuto::OnScanReset(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		_LivoxScanAuto *pV = (_LivoxScanAuto *)pPCV;
		pV->m_fProcess.set(pc_ScanReset);
	}

	void _LivoxScanAuto::OnScanSet(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		_LivoxScanAuto *pV = (_LivoxScanAuto *)pPCV;
		pV->m_fProcess.set(pc_ScanSet);
	}

	void _LivoxScanAuto::OnScanStart(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		_LivoxScanAuto *pV = (_LivoxScanAuto *)pPCV;
		pV->m_fProcess.set(pc_ScanStart);
	}

	void _LivoxScanAuto::OnScanStop(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		_LivoxScanAuto *pV = (_LivoxScanAuto *)pPCV;
		pV->m_fProcess.set(pc_ScanStop);
	}

	void _LivoxScanAuto::OnSavePC(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		_LivoxScanAuto *pV = (_LivoxScanAuto *)pPCV;
		pV->m_fProcess.set(pc_SavePC);
	}

	void _LivoxScanAuto::OnCamSet(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_LivoxScanAuto *pV = (_LivoxScanAuto *)pPCV;
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

	void _LivoxScanAuto::OnCamCtrl(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_LivoxScanAuto *pV = (_LivoxScanAuto *)pPCV;

		pV->m_camCtrl = *(VzCamCtrl *)pD;
		pV->m_fProcess.set(pc_CamCtrl);
	}
}
