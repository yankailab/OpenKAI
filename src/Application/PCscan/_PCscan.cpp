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
        m_pTk = NULL;
        m_pSB = NULL;
		m_bScanning = false;
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
        NULL__(m_pSB, -1);
		return this->_PCviewer::check();
	}

	void _PCscan::update(void)
	{
		m_pT->sleepT(0);

		while (m_nPread <= 0)
			readAllPC();

		m_spWin->AddPointCloud("PCMODEL", m_spPC);
		m_spWin->ResetCameraToDefault();

		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			if(m_bScanning)
			{
				readAllPC();
				m_spWin->UpdatePointCloud("PCMODEL", m_spPC);
			}

			m_pT->autoFPSto();
		}
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

        for (int i = 0; i < m_vpPCB.size(); i++)
        {
            _PCbase *pP = m_vpPCB[i];
            pP->setTranslation(m_pSB->mT());
        }
    }

	void _PCscan::updateUI(void)
	{
		auto &app = gui::Application::GetInstance();
		app.Initialize(m_pathRes.c_str());

		m_spWin = std::make_shared<visualizer::PCscanUI>(*this->getName(), 2000, 1000);
		app.AddWindow(m_spWin);

		m_spWin->SetCbBtnScan(OnBtnScan, (void *)this);
//		m_spWin->SetCbSavePC(OnBtnSavePC, (void *)this);

		m_pT->wakeUp();
		app.Run();
		exit(0);
	}

	void _PCscan::OnBtnScan(void *pPCV, void* pD)
	{
		NULL_(pPCV);
		_PCscan *pV = (_PCscan *)pPCV;


		if(pV->m_bScanning)
		{
		}

	}

	void _PCscan::OnBtnSavePC(void *pPCV, void* pD)
	{
		NULL_(pPCV);
		_PCscan *pV = (_PCscan *)pPCV;

		io::WritePointCloudOption par;
		par.write_ascii = io::WritePointCloudOption::IsAscii::Binary;
		par.compressed = io::WritePointCloudOption::Compressed::Uncompressed;

		pthread_mutex_lock(&pV->m_mutexPC);
		PointCloud pc = *pV->m_sPC.prev();
		pthread_mutex_unlock(&pV->m_mutexPC);
		io::WritePointCloudToPLY("/home/kai/dev/testModel.ply", pc, par);
	}

}
#endif
