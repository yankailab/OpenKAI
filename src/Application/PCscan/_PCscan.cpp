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
		m_bScanning = false;
		m_modelName = "PCMODEL";

		m_bSceneCache = false;
		m_selectPointSize = 0.025;

        pthread_mutex_init(&m_mutexScan, NULL);
	}

	_PCscan::~_PCscan()
	{
        pthread_mutex_destroy(&m_mutexScan);
		DEL(m_pTk);
	}

	bool _PCscan::init(void *pKiss)
	{
		IF_F(!this->_PCviewer::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("bSceneCache", &m_bSceneCache);
		pK->v("selectPointSize", &m_selectPointSize);

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

			updateScan();

			m_pT->autoFPSto();
		}
	}

	void _PCscan::updateScan(void)
	{
		IF_(check() < 0);
		IF_(!m_bScanning);

        pthread_mutex_lock(&m_mutexScan);

		readAllPC();
		PointCloud *pPC = m_sPC.get();
		pPC->normals_.clear();
		int n = pPC->points_.size();
		int nP = m_pPS->nP();

		m_aabb = pPC->GetAxisAlignedBoundingBox();

		PointCloud pc = *pPC;
		if (n < nP)
		{
			addDummyPC(&pc, nP - n, 10, 0, Vector3d(1, 0, 0));
		}
		else if (n > nP)
		{
			int d = n - nP;
			pc.points_.erase(pc.points_.end()-d, pc.points_.end());
			pc.colors_.erase(pc.colors_.end()-d, pc.colors_.end());
		}

		updateUIpc(pc);
		m_spWin->SetProgressBar((float)m_pPS->iP() / (float)m_pPS->nP());

        pthread_mutex_unlock(&m_mutexScan);
	}

	bool _PCscan::startScan(void)
	{
		IF_F(check() < 0);

        pthread_mutex_lock(&m_mutexScan);

		m_pSB->reset();
		sleep(1);
		m_pPS->clear();

		m_spWin->RemoveGeometry(m_modelName);

		addDummyPC(m_sPC.next(), m_pPS->nP(), 10, 0, Vector3d(1, 0, 0));
		updatePC();

		addUIpc(*m_sPC.get());
		m_aabb = m_sPC.get()->GetAxisAlignedBoundingBox();	

		m_bScanning = true;

        pthread_mutex_unlock(&m_mutexScan);
	}

	bool _PCscan::stopScan(void)
	{
        pthread_mutex_lock(&m_mutexScan);

		m_spWin->RemoveGeometry(m_modelName);

		addUIpc(*m_sPC.get());
		m_aabb = m_sPC.get()->GetAxisAlignedBoundingBox();

		m_bScanning = false;

        pthread_mutex_unlock(&m_mutexScan);
	}

	void _PCscan::addUIpc(const PointCloud& pc)
	{
		m_spWin->AddPointCloud(m_modelName,
							   make_shared<t::geometry::PointCloud>(
								   t::geometry::PointCloud::FromLegacyPointCloud(
									   pc,
									   core::Dtype::Float32)));		
	}

	void _PCscan::updateUIpc(const PointCloud& pc)
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
		IF_(!m_bScanning);

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
		m_spWin->SetCbBtnSavePC(OnBtnSavePC, (void *)this);
		m_spWin->SetCbBtnCamSet(OnBtnCamSet, (void *)this);

		visualizer::UIState* pU = m_spWin->getUIState();
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
		IF_(check()<0);
		IF_(!m_spWin);

		m_spWin->CamSetProj(m_camProj.m_fov,
							m_camProj.m_aspect,
							m_camProj.m_vNF.x,
							m_camProj.m_vNF.y,
							m_camProj.m_fovType);
	}

	void _PCscan::updateCamPose(void)
	{
		IF_(check()<0);
		IF_(!m_spWin);

		Vector3f vCenter(m_vCamCenter.x, m_vCamCenter.y, m_vCamCenter.z);
		Vector3f vEye(m_vCamEye.x, m_vCamEye.y, m_vCamEye.z);
		Vector3f vUp(m_vCamUp.x, m_vCamUp.y, m_vCamUp.z);
		m_spWin->CamSetPose(vCenter, vEye, vUp);
	}

	void _PCscan::camBound(const AxisAlignedBoundingBox& aabb)
	{
		IF_(check()<0);
		IF_(!m_spWin);

		Vector3f cor(m_vCamCoR.x, m_vCamCoR.y, m_vCamCoR.z);
		m_spWin->CamAutoBound(aabb, cor);
	}

	void _PCscan::OnBtnScan(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_PCscan *pV = (_PCscan *)pPCV;
		bool bScanning = *((bool*)pD);

		if (bScanning)
			pV->startScan();
		else
			pV->stopScan();
	}

	void _PCscan::OnBtnSavePC(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_PCscan *pV = (_PCscan *)pPCV;

		io::WritePointCloudOption par;
		par.write_ascii = io::WritePointCloudOption::IsAscii::Binary;
		par.compressed = io::WritePointCloudOption::Compressed::Uncompressed;

		pthread_mutex_lock(&pV->m_mutexPC);
		PointCloud pc = *pV->m_sPC.get();
		pthread_mutex_unlock(&pV->m_mutexPC);
		io::WritePointCloudToPLY((const char *)pD, pc, par);
	}

	void _PCscan::OnBtnCamSet(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_PCscan *pV = (_PCscan *)pPCV;
		bool bAuto = *(bool*)pD;

		if(!bAuto)
		{
			pV->updateCamPose();
			return;
		}

		pV->camBound(pV->m_aabb);
	}


}
#endif
