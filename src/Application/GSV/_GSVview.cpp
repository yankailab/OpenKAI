/*
 * _GSVview.cpp
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#include "_GSVview.h"

namespace kai
{
	_GSVview::_GSVview()
	{
		m_rVoxel = 0.1;
		m_fProcess.clearAll();

		m_fNameSavePC = "";
	}

	_GSVview::~_GSVview()
	{
	}

	bool _GSVview::init(void *pKiss)
	{
		IF_F(!this->_GeometryViewer::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("rVoxel", &m_rVoxel);

		return true;
	}

	bool _GSVview::link(void)
	{
		IF_F(!this->_GeometryViewer::link());

		Kiss *pK = (Kiss *)m_pKiss;

		// string n;
		// n = "";
		// F_ERROR_F(pK->v("_LivoxAutoScan", &n));
		// m_pLivox = (_LivoxAutoScan *)(pK->getInst(n));
		// NULL_Fl(m_pLivox, n + ": not found");

		return true;
	}

	bool _GSVview::start(void)
	{
		NULL_F(m_pT);
		IF_F(!m_pT->start(getUpdate, this));

		NULL_F(m_pTui);
		IF_F(!m_pTui->start(getUpdateUI, this));

		return true;
	}

	int _GSVview::check(void)
	{
		NULL__(m_pWin, -1);
		IF__(m_vpGB.empty(), -1);

		return this->_GeometryViewer::check();
	}

	void _GSVview::update(void)
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

	void _GSVview::scanReset(void)
	{
		IF_(check() < 0);

		m_fProcess.set(pc_Scanning);

		resetCamPose();
		updateCamPose();
	}

	void _GSVview::scanTake(void)
	{
		IF_(check() < 0);

		// // Add voxel down for preview
		// PointCloud pcVd = *pc.VoxelDownSample(m_rVoxel);
		// int nPvd = pcVd.points_.size();
		// for (i = 0; i < nPvd; i++)
		// {
		// 	m_pPCprv->points_[m_iPprv] = pcVd.points_[i];
		// 	m_pPCprv->colors_[m_iPprv] = pcVd.colors_[i];
		// 	m_iPprv++;
		// 	if (m_iPprv >= m_nPresv)
		// 		break;
		// }

		// int nDummy = m_nPresv - m_iPprv;
		// if (nDummy > 0)
		// {
		// 	m_pPCprv->points_.erase(m_pPCprv->points_.end() - nDummy,
		// 							m_pPCprv->points_.end());
		// 	m_pPCprv->colors_.erase(m_pPCprv->colors_.end() - nDummy,
		// 							m_pPCprv->colors_.end());
		// 	addDummyPoints(m_pPCprv, nDummy, m_rDummyDome);
		// }

		// m_aabb = pc.GetAxisAlignedBoundingBox();
		// if (m_pUIstate)
		// 	m_pUIstate->m_sMove = m_vDmove.constrain(m_aabb.Volume() * 0.0001);

		// updateUIpc(*m_pPCprv);
	}

	void _GSVview::updateScan(void)
	{
		IF_(check() < 0);

		// _PCframe *pPsrc = (_PCframe *)m_vpGB[0];
		// PointCloud pc;
		// pPsrc->copyTo(&pc);

		// int nPnew = pc.points_.size();
		// IF_(nPnew <= 0);

		// int iPw = m_iPprv;
		// for (int i = 0; i < nPnew; i++)
		// {
		// 	m_pPCprv->points_[iPw] = pc.points_[i];
		// 	m_pPCprv->colors_[iPw] = pc.colors_[i];

		// 	iPw++;
		// 	if (iPw >= m_nPresv)
		// 		break;
		// }

		// int nDummy = m_nPresv - iPw;
		// if (nDummy > 0)
		// {
		// 	m_pPCprv->points_.erase(m_pPCprv->points_.end() - nDummy,
		// 							m_pPCprv->points_.end());
		// 	m_pPCprv->colors_.erase(m_pPCprv->colors_.end() - nDummy,
		// 							m_pPCprv->colors_.end());
		// 	addDummyPoints(m_pPCprv, nDummy, m_rDummyDome);
		// }

		// m_aabb = pc.GetAxisAlignedBoundingBox();
		// if (m_pUIstate)
		// 	m_pUIstate->m_sMove = m_vDmove.constrain(m_aabb.Volume() * 0.0001);

		// updateUIpc(*m_pPCprv);
	}

	void _GSVview::savePC(void)
	{
		IF_(check() < 0);
		// if (m_nP <= 0)
		// {
		// 	m_pWin->ShowMsg("FILE", "Model is empty", true);
		// 	return;
		// }

		// m_pWin->ShowMsg("FILE", "Saving .PLY file");

		// io::WritePointCloudOption par;
		// par.write_ascii = io::WritePointCloudOption::IsAscii::Binary;
		// par.compressed = io::WritePointCloudOption::Compressed::Uncompressed;

		// PointCloud pcMerge;
		// int nSave = 0;
		// for (int i = 0; i < m_vPC.size(); i++)
		// {
		// 	PointCloud* pP = &m_vPC[i];
		// 	nSave += (io::WritePointCloudToPLY(m_fNameSavePC + i2str(i) + ".ply",
		// 									 *pP,
		// 									 par))?1:0;

		// 	pcMerge += *pP;
		// }

		// nSave += (io::WritePointCloudToPLY(m_fNameSavePC + "_merged.ply",
		// 								 pcMerge,
		// 								 par))?1:0;

		// m_pWin->CloseDialog();
		// string msg;
		// if (nSave > m_vPC.size())
		// {
		// 	msg = "Saved to: " + m_fNameSavePC;
		// 	m_pWin->ShowMsg("FILE", msg.c_str(), true);
		// }
		// else
		// {
		// 	msg = "Failed to save: " + m_fNameSavePC;
		// 	m_pWin->ShowMsg("FILE", msg.c_str(), true);
		// }
	}

	void _GSVview::updateCamAuto(void)
	{
		IF_(check() < 0);

		// Eigen::Affine3f A;
		// A = m_pNav->mT().cast<float>();
		// m_cam.m_vEye = A * m_camAuto.m_vEye.v3f();
		// m_cam.m_vLookAt = A * m_camAuto.m_vLookAt.v3f();

		updateCamPose();
	}

	void _GSVview::updateCamCtrl(void)
	{
		IF_(check() < 0);

		// _VzensePC *pVz = (_VzensePC *)m_vpGB[0];
		// NULL_(pVz);

		// pVz->setCamCtrl(m_camCtrl);
	}

	void _GSVview::updateUI(void)
	{
		auto &app = gui::Application::GetInstance();
		app.Initialize(m_pathRes.c_str());

		// m_pWin = new _GSVviewUI(*this->getName(), 2000, 1000);
		// m_pUIstate = m_pWin->getUIState();
		// m_pUIstate->m_bSceneCache = m_bSceneCache;
		// m_pUIstate->m_mouseMode = (visualization::gui::SceneWidget::Controls)m_mouseMode;
		// m_pUIstate->m_wPanel = m_wPanel;
		// m_pUIstate->m_sMove = m_vDmove.x;
		// m_pUIstate->m_btnPaddingH = m_vBtnPadding.x;
		// m_pUIstate->m_btnPaddingV = m_vBtnPadding.y;
		// m_pUIstate->m_dirSave = m_dirSave;
		// m_pWin->Init();
		// _GSVviewUI *pW = (_GSVviewUI *)m_pWin;
		// app.AddWindow(shared_ptr<_GSVviewUI>(pW));

		// pW->SetCbScanReset(OnScanReset, (void *)this);
		// pW->SetCbScanTake(OnScanTake, (void *)this);
		// pW->SetCbSavePC(OnSavePC, (void *)this);
		// pW->SetCbCamSet(OnCamSet, (void *)this);
		// pW->SetCbCamCtrl(OnCamCtrl, (void *)this);

		// m_pWin->UpdateUIstate();
		// m_pWin->SetFullScreen(m_bFullScreen);
		// m_aabb = createDefaultAABB();
		// camBound(m_aabb);
		// updateCamProj();
		// updateCamPose();

		// m_pT->run();
		// app.Run();
		// exit(0);
	}

	void _GSVview::OnScanReset(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		_GSVview *pV = (_GSVview *)pPCV;

		pV->m_fProcess.set(pc_ScanReset);
	}

	void _GSVview::OnScanTake(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		_GSVview *pV = (_GSVview *)pPCV;

		pV->m_fProcess.set(pc_ScanTake);
	}

	void _GSVview::OnSavePC(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		_GSVview *pV = (_GSVview *)pPCV;

		pV->m_fNameSavePC = *(string *)pD;
		pV->m_fProcess.set(pc_SavePC);
	}

	void _GSVview::OnCamSet(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_GSVview *pV = (_GSVview *)pPCV;
		int camMode = *(int *)pD;

		if (camMode == 0) // auto off
		{
			pV->m_fProcess.clear(pc_CamAuto);
		}
		else if (camMode == 1) // auto on
		{
			pV->m_fProcess.set(pc_CamAuto);
		}
		else if (camMode == 3) // origin or no point data
		{
			pV->resetCamPose();
			pV->updateCamPose();
		}
		else if (camMode == 4) // all
		{
			pV->camBound(pV->m_aabb);
		}
	}

	void _GSVview::OnCamCtrl(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_GSVview *pV = (_GSVview *)pPCV;

		// pV->m_camCtrl = *(VzCtrl *)pD;
		// pV->m_fProcess.set(pc_CamCtrl);
	}
}
