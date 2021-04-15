/*
 * _PCscan.cpp
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifdef USE_OPEN3D
#include "_PCscan.h"

namespace kai
{

	_PCscan::_PCscan()
	{
		m_pPS = NULL;
		m_pTk = NULL;
		m_pSB = NULL;
		m_pUIstate = NULL;
		m_modelName = "PCMODEL";

		m_bSceneCache = false;
		m_selectPointSize = 0.025;
		m_rDummyDome = 1000.0;
		m_dHiddenRemove = 100.0;

		m_fProcess.clearAll();
	}

	_PCscan::~_PCscan()
	{
		DEL(m_pTk);
	}

	bool _PCscan::init(void *pKiss)
	{
		IF_F(!this->_PCviewer::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("bSceneCache", &m_bSceneCache);
		pK->v("selectPointSize", &m_selectPointSize);
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
		NULL__(m_pPS, -1);
		NULL__(m_pSB, -1);

		return this->_PCviewer::check();
	}

	void _PCscan::update(void)
	{
		m_pT->sleepT(0);

		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			if (m_fProcess.b(pcfScanStart))
			{
				startScan();
			}

			if (m_fProcess.b(pcfScanStop))
			{
				stopScan();
			}

			if (m_fProcess.b(pcfScanning, false))
			{
				updateScan();
			}
			else
			{
				updateProcess();
			}

			m_pT->autoFPSto();
		}
	}

	void _PCscan::startScan(void)
	{
		IF_(check() < 0);

		m_spWin->ShowMsg("Scan", "Initializing");

		m_pSB->reset();
		sleep(1);
		m_pPS->clear();

		m_spWin->RemoveGeometry(m_modelName);
		addDummyDome(m_sPC.next(), m_pPS->nP(), m_rDummyDome);
		updatePC();
		addUIpc(*m_sPC.get());
		m_aabb = m_sPC.get()->GetAxisAlignedBoundingBox();
		m_fProcess.set(pcfScanning);

		m_spWin->CloseDialog();
	}

	void _PCscan::stopScan(void)
	{
		IF_(check() < 0);

		m_spWin->ShowMsg("Scan", "Processing");

		m_spWin->RemoveGeometry(m_modelName);
		m_aabb = m_sPC.get()->GetAxisAlignedBoundingBox();
		addUIpc(*m_sPC.get());
		m_fProcess.clear(pcfScanning);

		m_spWin->CloseDialog();
	}

	void _PCscan::updateScan(void)
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
		m_spWin->SetProgressBar((float)m_pPS->iP() / (float)m_pPS->nP());
	}

	void _PCscan::updateProcess(void)
	{
		IF_(check() < 0);

		if (m_fProcess.b(pcfResetPC))
		{
			m_spWin->RemoveGeometry(m_modelName);
			m_aabb = m_sPC.get()->GetAxisAlignedBoundingBox();
			addUIpc(*m_sPC.get());
		}

		if (m_fProcess.b(pcfVoxelDown) && m_pUIstate)
		{
			m_spWin->ShowMsg("Voxel Down Sampling", "Processing");

			m_spWin->RemoveGeometry(m_modelName);
			PointCloud pc;
			float s = m_pUIstate->m_voxelSize;
			if (s > 0.0)
				pc = *m_sPC.get()->VoxelDownSample(s);
			else
				pc = *m_sPC.get();

			m_aabb = pc.GetAxisAlignedBoundingBox();
			addUIpc(pc);
			m_spWin->CloseDialog();
		}

		if (m_fProcess.b(pcfHiddenRemove) && m_pUIstate)
		{
			m_spWin->ShowMsg("Hidden Point Removal", "Processing");

			m_spWin->RemoveGeometry(m_modelName);

			PointCloud pc = *m_sPC.get();
			auto pcR = pc.HiddenPointRemoval(m_pUIstate->m_vCamPos.cast<double>(), m_dHiddenRemove);
			pc = *pc.SelectByIndex(std::get<1>(pcR));

			addUIpc(pc);
			m_spWin->CloseDialog();
		}
	}

	void _PCscan::addUIpc(const PointCloud &pc)
	{
		m_spWin->AddPointCloud(m_modelName,
							   make_shared<t::geometry::PointCloud>(
								   t::geometry::PointCloud::FromLegacyPointCloud(
									   pc,
									   core::Dtype::Float32)));
	}

	void _PCscan::updateUIpc(const PointCloud &pc)
	{
		m_spWin->UpdatePointCloud(m_modelName,
								  make_shared<t::geometry::PointCloud>(
									  t::geometry::PointCloud::FromLegacyPointCloud(
										  pc,
										  core::Dtype::Float32)));
	}

	void _PCscan::updateKinematics(void)
	{
		while (m_pTk->bRun())
		{
			m_pTk->autoFPSfrom();

			updateSlam();

			m_pTk->autoFPSto();
		}
	}

	void _PCscan::updateSlam(void)
	{
		IF_(check() < 0);
		IF_(!m_fProcess.b(pcfScanning));

		auto mT = m_pSB->mT();
		for (int i = 0; i < m_vpPCB.size(); i++)
		{
			_PCbase *pP = m_vpPCB[i];
			pP->setTranslation(mT);
		}
	}

	void _PCscan::updateUI(void)
	{
		auto &app = gui::Application::GetInstance();
		app.Initialize(m_pathRes.c_str());

		m_spWin = std::make_shared<visualizer::PCscanUI>(*this->getName(), 2000, 1000);
		app.AddWindow(m_spWin);

		m_spWin->SetCbBtnScan(OnBtnScan, (void *)this);
		m_spWin->SetCbBtnOpenPC(OnBtnOpenPC, (void *)this);
		m_spWin->SetCbBtnCamSet(OnBtnCamSet, (void *)this);
		m_spWin->SetCbBtnHiddenRemove(OnBtnHiddenRemove, (void *)this);
		m_spWin->SetCbBtnResetPC(OnBtnResetPC, (void *)this);
		m_spWin->SetCbVoxelDown(OnVoxelDown, (void *)this);

		visualizer::UIState *pU = m_spWin->getUIState();
		pU->m_bSceneCache = m_bSceneCache;
		pU->m_selectPointSize = m_selectPointSize;
		m_spWin->UpdateUIstate();
		updateCamProj();
		updateCamPose();

		m_pT->wakeUp();
		app.Run();
		exit(0);
	}

	void _PCscan::updateCamProj(void)
	{
		IF_(check() < 0);
		IF_(!m_spWin);

		m_spWin->CamSetProj(m_camProj.m_fov,
							m_camProj.m_aspect,
							m_camProj.m_vNF.x,
							m_camProj.m_vNF.y,
							m_camProj.m_fovType);
	}

	void _PCscan::updateCamPose(void)
	{
		IF_(check() < 0);
		IF_(!m_spWin);

		Vector3f vCenter(m_vCamCenter.x, m_vCamCenter.y, m_vCamCenter.z);
		Vector3f vEye(m_vCamEye.x, m_vCamEye.y, m_vCamEye.z);
		Vector3f vUp(m_vCamUp.x, m_vCamUp.y, m_vCamUp.z);
		m_spWin->CamSetPose(vCenter, vEye, vUp);
	}

	void _PCscan::camBound(const AxisAlignedBoundingBox &aabb)
	{
		IF_(check() < 0);
		IF_(!m_spWin);

		Vector3f cor(m_vCamCoR.x, m_vCamCoR.y, m_vCamCoR.z);
		m_spWin->CamAutoBound(aabb, cor);
	}

	void _PCscan::OnBtnScan(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_PCscan *pV = (_PCscan *)pPCV;

		if (*((bool *)pD))
			pV->m_fProcess.set(pcfScanStart);
		else
			pV->m_fProcess.set(pcfScanStop);
	}

	void _PCscan::OnBtnOpenPC(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_PCscan *pV = (_PCscan *)pPCV;

		if (io::ReadPointCloud((const char *)pD, *pV->m_sPC.next()))
			pV->updatePC();

		PointCloud *pPC = pV->m_sPC.get();
		IF_(pPC->IsEmpty());
		pV->m_aabb = pPC->GetAxisAlignedBoundingBox();
		pV->addUIpc(*pPC);
	}

	void _PCscan::OnBtnCamSet(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_PCscan *pV = (_PCscan *)pPCV;
		bool bAuto = *(bool *)pD;

		if (!bAuto || pV->m_sPC.get()->points_.empty())
		{
			pV->updateCamPose();
			return;
		}

		pV->camBound(pV->m_aabb);
	}

	void _PCscan::OnBtnHiddenRemove(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_PCscan *pV = (_PCscan *)pPCV;
		pV->m_pUIstate = (visualizer::UIState *)pD;
		pV->m_fProcess.set(pcfHiddenRemove);
	}

	void _PCscan::OnVoxelDown(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_PCscan *pV = (_PCscan *)pPCV;
		pV->m_pUIstate = (visualizer::UIState *)pD;
		pV->m_fProcess.set(pcfVoxelDown);
	}

	void _PCscan::OnBtnResetPC(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		_PCscan *pV = (_PCscan *)pPCV;
		pV->m_fProcess.set(pcfResetPC);
	}

}
#endif
