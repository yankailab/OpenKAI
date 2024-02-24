/*
 * _VzScanCalib.cpp
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#include "_VzScanCalib.h"

namespace kai
{
	_VzScanCalib::_VzScanCalib()
	{
		m_pNav = NULL;
		m_pTk = NULL;

		m_nPC = 0;
		m_iPprv = 0;
		m_nPprv = 10000000;
		m_rVoxel = 0.1;
		m_fProcess.clearAll();

		m_fNameCalib = "";
	}

	_VzScanCalib::~_VzScanCalib()
	{
		DEL(m_pTk);
	}

	bool _VzScanCalib::init(void *pKiss)
	{
		IF_F(!this->_GeometryViewer::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("nPprv", &m_nPprv);
		pK->v("rVoxel", &m_rVoxel);
		pK->v("fNameCalib", &m_fNameCalib);

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

	bool _VzScanCalib::start(void)
	{
		NULL_F(m_pT);
		IF_F(!m_pT->start(getUpdate, this));

		NULL_F(m_pTk);
		IF_F(!m_pTk->start(getUpdateKinematics, this));

		NULL_F(m_pTui);
		IF_F(!m_pTui->start(getUpdateUI, this));

		return true;
	}

	int _VzScanCalib::check(void)
	{
		NULL__(m_pNav, -1);
		NULL__(m_pWin, -1);
		IF__(m_vpGB.empty(), -1);

		return this->_GeometryViewer::check();
	}

	void _VzScanCalib::update(void)
	{
		m_pT->sleepT(0);

		// init
		m_fProcess.set(pc_ScanReset);

		while (m_pT->bThread())
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

	void _VzScanCalib::scanReset(void)
	{
		IF_(check() < 0);

		m_pWin->ShowMsg("Scan", "Initializing");

		//		m_pNav->reset();

		m_nPC = 0;
		m_sPC.get()->Clear();
		m_sPC.next()->Clear();

		// point cloud for preview
		m_iPprv = 0;
		m_PCprv.Clear();
		addDummyPoints(&m_PCprv, m_nPprv, m_rDummyDome);

		removeUIpc();
		addUIpc(m_PCprv);
		m_fProcess.set(pc_Scanning);

		resetCamPose();
		updateCamPose();

		m_pWin->CloseDialog();
	}

	void _VzScanCalib::scanTake(void)
	{
		IF_(check() < 0);

		_PCframe *pPsrc = (_PCframe *)m_vpGB[0];
		PointCloud pc;
		pPsrc->getPC(&pc);
		IF_(pc.IsEmpty());
		int i;

		// Add original
		*m_sPC.next() = pc;
		m_sPC.swap();
		m_nPC++;

		// Merge for preview
		m_PCprv = *m_sPC.get();
		if (m_nPC >= 2)
			m_PCprv += *m_sPC.next();

		int nDummy = m_nPprv - m_iPprv;
		if (nDummy > 0)
		{
			addDummyPoints(&m_PCprv, nDummy, m_rDummyDome);
		}
		else
		{
			m_PCprv.points_.erase(m_PCprv.points_.end() - nDummy,
								  m_PCprv.points_.end());
			m_PCprv.colors_.erase(m_PCprv.colors_.end() - nDummy,
								  m_PCprv.colors_.end());
		}

		m_aabb = pc.GetAxisAlignedBoundingBox();
		if (m_pUIstate)
			m_pUIstate->m_sMove = m_vDmove.constrain(m_aabb.Volume() * 0.0001);

		updateUIpc(m_PCprv);

		// TODO: calib
	}

	void _VzScanCalib::updateScan(void)
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
			m_PCprv.points_[iPw] = pc.points_[i];
			m_PCprv.colors_[iPw] = pc.colors_[i];

			iPw++;
			if (iPw >= m_nPprv)
				break;
		}

		int nDummy = m_nPprv - iPw;
		if (nDummy > 0)
		{
			m_PCprv.points_.erase(m_PCprv.points_.end() - nDummy,
								  m_PCprv.points_.end());
			m_PCprv.colors_.erase(m_PCprv.colors_.end() - nDummy,
								  m_PCprv.colors_.end());
			addDummyPoints(&m_PCprv, nDummy, m_rDummyDome);
		}

		m_aabb = pc.GetAxisAlignedBoundingBox();
		if (m_pUIstate)
			m_pUIstate->m_sMove = m_vDmove.constrain(m_aabb.Volume() * 0.0001);

		updateUIpc(m_PCprv);
	}

	void _VzScanCalib::savePC(void)
	{
		IF_(check() < 0);
		// if (m_nP <= 0)
		// {
		// 	m_pWin->ShowMsg("FILE", "Model is empty", true);
		// 	return;
		// }

		// m_pWin->ShowMsg("FILE", "Saving .PLY file");


		// m_pWin->CloseDialog();
		// string msg;
		// if (nSave > m_vPC.size())
		// {
		// 	msg = "Saved to: " + m_fNameCalib;
		// 	m_pWin->ShowMsg("FILE", msg.c_str(), true);
		// }
		// else
		// {
		// 	msg = "Failed to save: " + m_fNameCalib;
		// 	m_pWin->ShowMsg("FILE", msg.c_str(), true);
		// }
	}

	void _VzScanCalib::updateCamAuto(void)
	{
		IF_(check() < 0);

		Eigen::Affine3f A;
		A = m_pNav->mT().cast<float>();
		m_cam.m_vEye = A * m_camAuto.m_vEye.v3f();
		m_cam.m_vLookAt = A * m_camAuto.m_vLookAt.v3f();

		updateCamPose();
	}

	void _VzScanCalib::updateCamCtrl(void)
	{
		IF_(check() < 0);

		_VzensePC *pVz = (_VzensePC *)m_vpGB[0];
		NULL_(pVz);

		pVz->setCamCtrl(m_camCtrl);
	}

	void _VzScanCalib::updateKinematics(void)
	{
		while (m_pTk->bRun())
		{
			m_pTk->autoFPSfrom();

			updateSlam();

			m_pTk->autoFPSto();
		}
	}

	void _VzScanCalib::updateSlam(void)
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

	void _VzScanCalib::updateUI(void)
	{
		auto &app = gui::Application::GetInstance();
		app.Initialize(m_pathRes.c_str());

		m_pWin = new _VzScanCalibUI(*this->getName(), 2000, 1000);
		m_pUIstate = m_pWin->getUIState();
		m_pUIstate->m_bSceneCache = m_bSceneCache;
		m_pUIstate->m_mouseMode = (visualization::gui::SceneWidget::Controls)m_mouseMode;
		m_pUIstate->m_wPanel = m_wPanel;
		m_pUIstate->m_sMove = m_vDmove.x;
		m_pUIstate->m_btnPaddingH = m_vBtnPadding.x;
		m_pUIstate->m_btnPaddingV = m_vBtnPadding.y;
		m_pUIstate->m_dirSave = m_dirSave;
		m_pWin->Init();
		_VzScanCalibUI *pW = (_VzScanCalibUI *)m_pWin;
		app.AddWindow(shared_ptr<_VzScanCalibUI>(pW));

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

		m_pT->run();
		app.Run();
		exit(0);
	}

	void _VzScanCalib::OnScanReset(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		_VzScanCalib *pV = (_VzScanCalib *)pPCV;

		pV->m_fProcess.set(pc_ScanReset);
	}

	void _VzScanCalib::OnScanTake(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		_VzScanCalib *pV = (_VzScanCalib *)pPCV;

		pV->m_fProcess.set(pc_ScanTake);
	}

	void _VzScanCalib::OnSavePC(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		_VzScanCalib *pV = (_VzScanCalib *)pPCV;

		pV->m_fNameCalib = *(string *)pD;
		pV->m_fProcess.set(pc_SavePC);
	}

	void _VzScanCalib::OnOpenPC(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_VzScanCalib *pV = (_VzScanCalib *)pPCV;

		if (io::ReadPointCloud((const char *)pD, *pV->m_sPC.next()))
			pV->updatePC();

		pV->m_fProcess.set(pc_ResetPC);
	}

	void _VzScanCalib::OnCamSet(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_VzScanCalib *pV = (_VzScanCalib *)pPCV;
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

	void _VzScanCalib::OnCamCtrl(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_VzScanCalib *pV = (_VzScanCalib *)pPCV;

		pV->m_camCtrl = *(VzCtrl *)pD;
		pV->m_fProcess.set(pc_CamCtrl);
	}
}
