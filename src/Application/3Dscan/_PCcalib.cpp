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
		m_pCC = (_CamCalib *)(pK->findModule(n));

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
//		NULL__(m_pR, -1);
		NULL__(m_pCC, -1);

		return this->_PCscan::check();
	}

	void _PCcalib::update(void)
	{
		m_pT->sleepT(0);

		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			if (m_fProcess.b(pc_ScanStart, true))
			{
				startScan();
			}

			if (m_fProcess.b(pc_ScanStop, true))
			{
				stopScan();
			}

			if (m_fProcess.b(pc_RefreshCol, true))
			{
//				m_pPS->refreshCol();
			}

			if (m_fProcess.b(pc_Scanning))
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

	void _PCcalib::updateUI(void)
	{
		auto &app = gui::Application::GetInstance();
		app.Initialize(m_pathRes.c_str());

		m_pWin = new PCcalibUI(this->getName(), 2000, 1000);
		PCcalibUI *pW = (PCcalibUI *)m_pWin;
		app.AddWindow(shared_ptr<PCcalibUI>(pW));

		pW->SetCbScan(OnScan, (void *)this);
		pW->SetCbResetPC(OnResetPC, (void *)this);
		pW->SetCbRefreshCol(OnRefreshCol, (void *)this);
		pW->SetCbLoadImgs(OnLoadImgs, (void *)this);
		pW->SetCbUpdateParams(OnUpdateParams, (void *)this);
		pW->SetCbImportParams(OnImportParams, (void *)this);
		pW->SetCbExportParams(OnExportParams, (void *)this);

		m_pUIstate = m_pWin->getUIState();
		m_pUIstate->m_bSceneCache = m_bSceneCache;
		m_pUIstate->m_mouseMode = (visualization::gui::SceneWidget::Controls)m_mouseMode;
		m_pUIstate->m_wPanel = m_wPanel;
		m_pUIstate->m_sMove = m_vDmove.x;
		m_pWin->UpdateUIstate();
//		m_pWin->SetFullScreen(m_bFullScreen);
		m_aabb = createDefaultAABB();
		camBound(m_aabb);
		updateCamProj();
		updateCamPose();

		m_pT->run();
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

	void _PCcalib::OnRefreshCol(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		_PCcalib *pV = (_PCcalib *)pPCV;
		
		pV->m_fProcess.set(pc_RefreshCol);
	}

	void _PCcalib::OnUpdateParams(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		_PCcalib *pV = (_PCcalib *)pPCV;

		pV->updateParams();
	}

	void _PCcalib::OnImportParams(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_PCcalib *pV = (_PCcalib *)pPCV;

		pV->importParams((char *)pD);
	}

	void _PCcalib::OnExportParams(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_PCcalib *pV = (_PCcalib *)pPCV;

		pV->exportParams((char *)pD);
	}

	void _PCcalib::importParams(const char *pPath)
	{
		NULL_(pPath);
		string fKiss = string(pPath);
		IF_(fKiss.empty());

		_File *pF = new _File();
		IF_(!pF->open(fKiss, ios::in));
		string f;
		IF_(!pF->readAll(&f));
		IF_(f.empty());
		pF->close();
		DEL(pF);

		Kiss *pKf = new Kiss();
		IF_(!pKf->parse(&f));
		Kiss* pK = pKf->child("calib");

		//update to UI
		PCcalibUI *pW = (PCcalibUI *)m_pWin;
		PCCALIB_PARAM *pP = pW->GetCalibParams();

		pK->v("Fx", &pP->m_Fx);
		pK->v("Fy", &pP->m_Fy);
		pK->v("Cx", &pP->m_Cx);
		pK->v("Cy", &pP->m_Cy);

		pK->v("k1", &pP->m_k1);
		pK->v("k2", &pP->m_k2);
		pK->v("p1", &pP->m_p1);
		pK->v("p2", &pP->m_p2);
		pK->v("k3", &pP->m_k3);

		vDouble3 v;
		pK->v("vOffsetPt", &v);
		pP->m_poTx = v.x;
		pP->m_poTy = v.y;
		pP->m_poTz = v.z;
		pK->v("vOffsetPr", &v);
		pP->m_poRx = v.x;
		pP->m_poRy = v.y;
		pP->m_poRz = v.z;
		pK->v("vOffsetCt", &v);
		pP->m_coTx = v.x;
		pP->m_coTy = v.y;
		pP->m_coTz = v.z;
		pK->v("vOffsetCr", &v);
		pP->m_coRx = v.x;
		pP->m_coRy = v.y;
		pP->m_coRz = v.z;

		delete pKf;
	
		pW->UpdateCalibParams();
		updateParams();
	}

	void _PCcalib::exportParams(const char *pPath)
	{
		NULL_(pPath);
		string fKiss = string(pPath);
		IF_(fKiss.empty());

		updateParams();
		PCcalibUI *pW = (PCcalibUI *)m_pWin;
		PCCALIB_PARAM *pP = pW->GetCalibParams();

		picojson::object o;
		o.insert(make_pair("name", "calib"));
		o.insert(make_pair("Fx", value(pP->m_Fx)));
		o.insert(make_pair("Fy", value(pP->m_Fy)));
		o.insert(make_pair("Cx", value(pP->m_Cx)));
		o.insert(make_pair("Cy", value(pP->m_Cy)));

		o.insert(make_pair("k1", value(pP->m_k1)));
		o.insert(make_pair("k2", value(pP->m_k2)));
		o.insert(make_pair("p1", value(pP->m_p1)));
		o.insert(make_pair("p2", value(pP->m_p2)));
		o.insert(make_pair("k3", value(pP->m_k3)));

		picojson::array v;
		v.push_back(value(pP->m_poTx));
		v.push_back(value(pP->m_poTy));
		v.push_back(value(pP->m_poTz));
		o.insert(make_pair("vOffsetPt", value(v)));
		v.clear();
		v.push_back(value(pP->m_poRx));
		v.push_back(value(pP->m_poRy));
		v.push_back(value(pP->m_poRz));
		o.insert(make_pair("vOffsetPr", value(v)));
		v.clear();
		v.push_back(value(pP->m_coTx));
		v.push_back(value(pP->m_coTy));
		v.push_back(value(pP->m_coTz));
		o.insert(make_pair("vOffsetCt", value(v)));
		v.clear();
		v.push_back(value(pP->m_coRx));
		v.push_back(value(pP->m_coRy));
		v.push_back(value(pP->m_coRz));
		o.insert(make_pair("vOffsetCr", value(v)));
		v.clear();

		string f = picojson::value(o).serialize();

		_File *pF = new _File();
		IF_(!pF->open(fKiss, ios::out));
		pF->write((uint8_t *)f.c_str(), f.length());
		pF->close();
		DEL(pF);
	}

	bool _PCcalib::calibRGB(const char *pPath)
	{
		IF_F(check() < 0);
		NULL_F(pPath);

		IF_F(!m_pCC->calibRGB(pPath));

		PCcalibUI *pW = (PCcalibUI *)m_pWin;
		PCCALIB_PARAM *pP = pW->GetCalibParams();
		Mat mC = m_pCC->mC();
		Mat mD = m_pCC->mD();

		pP->m_Fx = mC.at<double>(0, 0);
		pP->m_Fy = mC.at<double>(1, 1);
		pP->m_Cx = mC.at<double>(0, 2);
		pP->m_Cy = mC.at<double>(1, 2);

		pP->m_k1 = mD.at<double>(0, 0);
		pP->m_k2 = mD.at<double>(0, 1);
		pP->m_p1 = mD.at<double>(0, 2);
		pP->m_p2 = mD.at<double>(0, 3);
		pP->m_k3 = mD.at<double>(0, 4);

		pW->UpdateCalibParams();
		updateParams();
		return true;
	}

	void _PCcalib::updateParams(void)
	{
		//get data from UI and update the remap
		Mat mC = Mat::zeros(3, 3, CV_64FC1);
		Mat mD = Mat::zeros(1, 5, CV_64FC1);

		PCcalibUI *pW = (PCcalibUI *)m_pWin;
		PCCALIB_PARAM *pP = pW->GetCalibParams();
		mC.at<double>(0, 0) = pP->m_Fx;
		mC.at<double>(1, 1) = pP->m_Fy;
		mC.at<double>(0, 2) = pP->m_Cx;
		mC.at<double>(1, 2) = pP->m_Cy;
		mC.at<double>(2, 2) = 1.0;

		mD.at<double>(0, 0) = pP->m_k1;
		mD.at<double>(0, 1) = pP->m_k2;
		mD.at<double>(0, 2) = pP->m_p1;
		mD.at<double>(0, 3) = pP->m_p2;
		mD.at<double>(0, 4) = pP->m_k3;

//		m_pR->setCamMatrices(mC, mD);

		//set offset
		// NULL_(m_pPS);
		// vDouble3 vT,vR;
		
		// vT.init(pP->m_poTx, pP->m_poTy, pP->m_poTz);
		// vR.init(pP->m_poRx, pP->m_poRy, pP->m_poRz);
		// m_pPS->setOffset(vT, vR);

		// vT.init(pP->m_coTx, pP->m_coTy, pP->m_coTz);
		// vR.init(pP->m_coRx, pP->m_coRy, pP->m_coRz);
		// m_pPS->setRGBoffset(vT, vR);
	}
}
#endif
