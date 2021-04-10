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
	}

	_PCscan::~_PCscan()
	{
		DEL(m_pTk);
	}

	bool _PCscan::init(void *pKiss)
	{
		IF_F(!this->_PCviewer::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

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

		while (m_nPread < m_pPS->nP())
			readAllPC();

		std::shared_ptr<t::geometry::PointCloud> spTpc(new t::geometry::PointCloud());
		*spTpc = open3d::t::geometry::PointCloud::FromLegacyPointCloud(*m_spPC, core::Dtype::Float32);

		m_spWin->AddPointCloud(m_modelName, spTpc);
		m_spWin->ResetCameraToDefault();

//float a = 0.0;

		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			if (m_bScanning)
			{
//				readAllPC();

		//nP in the Update must be <= nP in the Add

		PointCloud pc;
        for (int i = 0; i < m_pPS->nP(); i++)
        {
            PC_POINT *pP = &m_pPS->m_pP[i];
            IF_CONT(pP->m_tStamp <= 0);

            pc.points_.push_back(pP->m_vP);
            pc.colors_.push_back(pP->m_vC);
        }

//		int nP = pc.points_.size();

		// PointCloud pc;
        // for (int i = 0; i < 1000; i++)
        // {
		// 	float f = (float)i;
		// 	f *= 0.1;
        //     pc.points_.push_back(Vector3d(f + a,f,f));
        //     pc.colors_.push_back(Vector3d(0,1,0));
        // }
		// a += 0.1;
		// if(a>10.0)a=0.0;

				std::shared_ptr<t::geometry::PointCloud> spTpc = 
					make_shared<t::geometry::PointCloud>(
						t::geometry::PointCloud::FromLegacyPointCloud(
							pc,
							core::Dtype::Float32
							)
						);

				m_spWin->UpdatePointCloud(m_modelName, spTpc);

				float m = 0.0;
				if (m_pPS)
					m = (float)m_pPS->iP() / (float)m_pPS->nP();
				m_spWin->SetProgressBar(m);
			}

			m_pT->autoFPSto();
		}
	}

	void _PCscan::updateKinematics(void)
	{
		while (m_pTk->bRun())
		{
			m_pTk->autoFPSfrom();

			if (m_bScanning)
			{
				updateSlam();
			}

			m_pTk->autoFPSto();
		}
	}

	void _PCscan::updateSlam(void)
	{
		IF_(check() < 0);

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

		m_pT->wakeUp();
		app.Run();
		exit(0);
	}

	bool _PCscan::startScan(void)
	{
		IF_F(check() < 0);
		m_bScanning = false;

		m_pSB->reset();
		sleep(1);
		m_pPS->clear();
		m_bScanning = true;
	}

	bool _PCscan::stopScan(void)
	{
		m_bScanning = false;
	}

	void _PCscan::OnBtnScan(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_PCscan *pV = (_PCscan *)pPCV;
		bool bScanning = *((int *)pD) != 0 ? true : false;

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
		PointCloud pc = *pV->m_sPC.prev();
		pthread_mutex_unlock(&pV->m_mutexPC);
		io::WritePointCloudToPLY((const char *)pD, pc, par);
	}

}
#endif
