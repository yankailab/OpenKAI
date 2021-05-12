/*
 * _PCcalib.cpp
 *
 *  Created on: May 6, 2021
 *      Author: yankai
 */

#ifdef USE_OPEN3D
#include "_PCcalib.h"

namespace kai
{

	_PCcalib::_PCcalib()
	{
		m_bFullScreen = false;
		m_mouseMode = 0;
		m_rDummyDome = 1000.0;

		m_fProcess.clearAll();
	}

	_PCcalib::~_PCcalib()
	{
	}

	bool _PCcalib::init(void *pKiss)
	{
		IF_F(!this->_PCscan::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		string n;
		n = "";
		pK->v("_CamCalib", &n);
		m_pCC = (_CamCalib *)(pK->getInst(n));

		return true;
	}

	bool _PCcalib::start(void)
	{
		NULL_F(m_pT);
		IF_F(!m_pT->start(getUpdate, this));

		NULL_F(m_pTk);
		IF_F(!m_pTk->start(getUpdateKinematics, this));

		NULL_F(m_pTui);
		IF_F(!m_pTui->start(getUpdateUI, this));

		return true;
	}

	int _PCcalib::check(void)
	{
		NULL__(m_pV, -1);
		NULL__(m_pCC, -1);

		return this->_PCscan::check();
	}

	void _PCcalib::update(void)
	{
		m_pT->sleepT(0);

		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			if (m_fProcess.b(pc_ScanStart))
			{
				startScan();
			}

			if (m_fProcess.b(pc_ScanStop))
			{
				stopScan();
			}

			if (m_fProcess.b(pc_Scanning, false))
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

	void _PCcalib::updateProcess(void)
	{
		IF_(check() < 0);

		if (m_fProcess.b(pc_ResetPC))
		{
			removeUIpc();
			addUIpc(*m_sPC.get());
			m_aabb = m_sPC.get()->GetAxisAlignedBoundingBox();
			camBound(m_aabb);
		}

		if (m_fProcess.b(pc_VoxelDown) && m_pUIstate)
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

		if (m_fProcess.b(pc_HiddenRemove) && m_pUIstate)
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

	void _PCcalib::updateUI(void)
	{
		auto &app = gui::Application::GetInstance();
		app.Initialize(m_pathRes.c_str());

		m_pWin = new PCcalibUI(*this->getName(), 2000, 1000);
		PCcalibUI* pW = (PCcalibUI*)m_pWin;
		app.AddWindow(shared_ptr<PCcalibUI>(pW));

		pW->SetCbScan(OnBtnScan, (void *)this);
		pW->SetCbResetPC(OnBtnResetPC, (void *)this);
		pW->SetCbLoadImgs(OnLoadImgs, (void *)this);
		pW->SetCbUpdateParams(OnUpdateParams, (void *)this);

		m_pUIstate = m_pWin->getUIState();
		m_pUIstate->m_bSceneCache = m_bSceneCache;
		m_pUIstate->m_mouseMode = (visualization::gui::SceneWidget::Controls)m_mouseMode;
		m_pUIstate->m_wPanel = m_wPanel;
		m_pUIstate->m_sMove = m_vDmove.x;
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

	void _PCcalib::OnLoadImgs(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_PCcalib *pV = (_PCcalib *)pPCV;

		pV->calibRGB((const char *)pD);
	}

	void _PCcalib::OnUpdateParams(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		_PCcalib *pV = (_PCcalib *)pPCV;

		pV->updateParams();
	}

	bool _PCcalib::calibRGB(const char *pPath)
	{
		IF_F(check() < 0);
		NULL_F(pPath);

		IF_F(!m_pCC->calibRGB(pPath));
		Mat mC = m_pCC->camMatrix();
		Mat mD = m_pCC->distCoeffs();

		//update to UI
		PCcalibUI* pW = (PCcalibUI*)m_pWin;
		PCCALIB_PARAM* pP = pW->GetCalibParams();
		pP->m_Fx = mC.at<double>(0,0);
		pP->m_Fy = mC.at<double>(1,1);
		pP->m_Cx = mC.at<double>(0,2);
		pP->m_Cy = mC.at<double>(1,2);

		pP->m_k1 = mD.at<double>(0,0);
		pP->m_k2 = mD.at<double>(0,1);
		pP->m_p1 = mD.at<double>(0,2);
		pP->m_p2 = mD.at<double>(0,3);
		pP->m_k3 = mD.at<double>(0,4);
		pW->UpdateCalibParams();

		//update self
		updateParams();

		return true;
	}

	void _PCcalib::updateParams(void)
	{
		//update from UI
		Mat mC = Mat::zeros(3,3,CV_64FC1);
		Mat mD = Mat::zeros(1,5,CV_64FC1);

		PCcalibUI* pW = (PCcalibUI*)m_pWin;
		PCCALIB_PARAM* pP = pW->GetCalibParams();
		mC.at<double>(0,0) = pP->m_Fx;
		mC.at<double>(1,1) = pP->m_Fy;
		mC.at<double>(0,2) = pP->m_Cx;
		mC.at<double>(1,2) = pP->m_Cy;

		mD.at<double>(0,0) = pP->m_k1;
		mD.at<double>(0,1) = pP->m_k2;
		mD.at<double>(0,2) = pP->m_p1;
		mD.at<double>(0,3) = pP->m_p2;
		mD.at<double>(0,4) = pP->m_k3;

//		m_pVremap->setCamMatrices(mC, mD);
	}
}
#endif
