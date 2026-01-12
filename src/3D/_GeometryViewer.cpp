/*
 * _GeometryViewer.cpp
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#include "_GeometryViewer.h"

namespace kai
{

	_GeometryViewer::_GeometryViewer()
	{
		m_vWinSize.set(1280, 720);

		m_pTui = nullptr;
		m_pathRes = "";
		m_device = "CPU:0";
		m_vCoR.set(0, 0, 0);

		m_pWin = nullptr;
		m_pUIstate = nullptr;
		m_dirSave = "/home/lab/";

		m_bFullScreen = false;
		m_bSceneCache = false;
		m_wPanel = 15;
		m_mouseMode = 0;
		m_vDmove.set(0.5, 5.0);
	}

	_GeometryViewer::~_GeometryViewer()
	{
		DEL(m_pWin);
	}


    bool _GeometryViewer::init(const json &j)
    {
        IF_F(!this->_GeometryBase::init(j));

		m_vWinSize = j.value("vWinSize", vector<int>{1280, 720});
		m_pathRes = j.value("pathRes", "");
		m_device = j.value("device", "CPU:0");
		m_dirSave = j.value("dirSave", "/home/lab/");

		m_bFullScreen = j.value("bFullScreen", false);
		m_bSceneCache = j.value("bSceneCache", false);
		m_wPanel = j.value("wPanel", 15);
		m_vBtnPadding = j.value("vBtnPadding", vector<float>{0,0});
		m_mouseMode = j.value("mouseMode", 0);
		m_vDmove = j.value("vDmove", vector<float>{0.5, 5.0});

		m_camProj.m_type = j.value("camProjType", 0);
		m_camProj.m_fov = j.value("camFov", 70.0);
		m_camProj.m_fovType = j.value("camFovType", 0);
		m_camProj.m_vLR = j.value("vCamLR", vector<float>{-10,10});
		m_camProj.m_vBT = j.value("vCamBT", vector<float>{-10,10});
		m_camProj.m_vNF = j.value("vCamNF", vector<float>{0, FLT_MAX});

		m_camDefault.m_vLookAt = j.value("vCamLookAt", vector<float>{0,0,0});
		m_camDefault.m_vEye = j.value("vCamEye", vector<float>{0,0,1});
		m_camDefault.m_vUp = j.value("vCamUp", vector<float>{0,1,0});
		m_cam = m_camDefault;

		m_camAuto.m_vLookAt = j.value("vCamAutoLookAt", vector<float>{0,0,0});
		m_camAuto.m_vEye = j.value("vCamAutoEye", vector<float>{0,0,1});
		m_camAuto.m_vUp = j.value("vCamAutoUp", vector<float>{0,1,0});

		m_vCoR = j.value("vCoR", vector<float>{0,0,0});

		utility::SetVerbosityLevel(utility::VerbosityLevel::Error);

        if (!j.contains("threadUI"))
        {
            LOG_E("json: threadUI not found");
            return false;
        }

        const json &jt = j.at("threadUI");
        if (!jt.is_object())
        {
            LOG_E("json: threadUI is not an object");
            return false;
        }

        DEL(m_pTui);
        m_pTui = new _Thread();
        if (!m_pTui->init(jt))
        {
            DEL(m_pTui);
            LOG_E("threadUI.init() failed");
            return false;
        }

        return true;
    }

    bool _GeometryViewer::link(const json &j, ModuleMgr *pM)
    {
        IF_F(!this->_GeometryBase::link(j, pM));

		const json& jg = j.at("geometry");
		IF__(!jg.is_object(), true);

		for (auto it = jg.begin(); it != jg.end(); it++)
		{
			const json &ji = it.value();
			IF_CONT(!ji.is_object());

			string n = ji.value("_GeometryBase", "");
			_GeometryBase *pGB = (_GeometryBase *)(pM->findModule(n));
			IF_CONT(!pGB);

			GVIEWER_OBJ g;
			g.m_pGB = pGB;
			g.m_name = n;
			g.m_bStatic = ji.value("bStatic", true);
			g.m_nPbuf = ji.value("nP", 0);
			g.m_rDummyDome = ji.value("rDummyDome", 1000.0);
			g.m_matName = ji.value("matName", "");
			g.m_matCol = ji.value("matCol", vector<float>{1,1,1,1});
			g.m_matPointSize = ji.value("matPointSize", 1);
			g.m_matLineWidth = ji.value("matLineWidth", 1);
			g.m_iGridLS = ji.value("iGridLS", 0);

			g.init();
			g.updateMaterial();

			m_vGO.push_back(g);
		}

        return true;
    }




	int _GeometryViewer::init(void *pKiss)
	{
		CHECK_(this->_GeometryBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vWinSize", &m_vWinSize);
		pK->v("pathRes", &m_pathRes);
		pK->v("device", &m_device);
		pK->v("dirSave", &m_dirSave);

		pK->v("bFullScreen", &m_bFullScreen);
		pK->v("bSceneCache", &m_bSceneCache);
		pK->v("wPanel", &m_wPanel);
		pK->v("vBtnPadding", &m_vBtnPadding);
		pK->v("mouseMode", &m_mouseMode);
		pK->v("vDmove", &m_vDmove);

		pK->v("camProjType", &m_camProj.m_type);
		pK->v("camFov", &m_camProj.m_fov);
		pK->v("camFovType", &m_camProj.m_fovType);
		pK->v("vCamLR", &m_camProj.m_vLR);
		pK->v("vCamBT", &m_camProj.m_vBT);
		pK->v("vCamNF", &m_camProj.m_vNF);

		pK->v("vCamLookAt", &m_camDefault.m_vLookAt);
		pK->v("vCamEye", &m_camDefault.m_vEye);
		pK->v("vCamUp", &m_camDefault.m_vUp);
		m_cam = m_camDefault;

		pK->v("vCamAutoLookAt", &m_camAuto.m_vLookAt);
		pK->v("vCamAutoEye", &m_camAuto.m_vEye);
		pK->v("vCamAutoUp", &m_camAuto.m_vUp);

		pK->v("vCoR", &m_vCoR);

		utility::SetVerbosityLevel(utility::VerbosityLevel::Error);

		Kiss *pKt = pK->child("threadUI");
		IF__(pKt->empty(), OK_ERR_NOT_FOUND);

		m_pTui = new _Thread();
		int r = m_pTui->init(pKt);
		if (r != OK_OK)
		{
			DEL(m_pTui);
			LOG_E("ThreadUI init failed");
			return r;
		}

		return OK_OK;
	}

	int _GeometryViewer::link(void)
	{
		CHECK_(this->_GeometryBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		Kiss *pKg = pK->child("geometry");
		IF__(pKg->empty(), true);

		int i = 0;
		while (1)
		{
			Kiss *pG = pKg->child(i++);
			if (pG->empty())
				break;

			string n;
			pG->v("_GeometryBase", &n);
			_GeometryBase *pGB = (_GeometryBase *)(pK->findModule(n));
			IF_CONT(!pGB);

			GVIEWER_OBJ g;
			g.m_pGB = pGB;
			g.m_name = n;
			pG->v("bStatic", &g.m_bStatic);
			pG->v("nP", &g.m_nPbuf);
			pG->v("rDummyDome", &g.m_rDummyDome);
			pG->v("matName", &g.m_matName);
			pG->v("matCol", &g.m_matCol);
			pG->v("matPointSize", &g.m_matPointSize);
			pG->v("matLineWidth", &g.m_matLineWidth);
			pG->v("iGridLS", &g.m_iGridLS);

			g.init();
			g.updateMaterial();

			m_vGO.push_back(g);
		}

		return OK_OK;
	}

	int _GeometryViewer::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		CHECK_(m_pT->start(getUpdate, this));

		NULL__(m_pTui, OK_ERR_NULLPTR);
		CHECK_(m_pTui->start(getUpdateUI, this));

		return OK_OK;
	}

	int _GeometryViewer::check(void)
	{
		return this->_GeometryBase::check();
	}

	void _GeometryViewer::update(void)
	{
		// wait for the UI thread to get window ready
		m_pT->sleepT(USEC_1SEC);

		while (!addAllGeometries())
			sleep(1);

		resetCamPose();
		updateCamPose();

		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateAllGeometries();
		}
	}

	bool _GeometryViewer::addAllGeometries(void)
	{
		IF_F(check() != OK_OK);
		IF_F(!m_pWin);

		for (int i = 0; i < m_vGO.size(); i++)
		{
			GVIEWER_OBJ *pG = &m_vGO[i];
			pG->updateGeometry();

			GEOMETRY_TYPE gt = pG->m_pGB->getType();
			switch (gt)
			{
			case pc_stream:
				m_pWin->RemoveGeometry(pG->m_name);
				m_pWin->AddPointCloud(pG->m_name, &pG->m_tPC, &pG->m_mat);
				break;
			case pc_frame:
				m_pWin->RemoveGeometry(pG->m_name);
				m_pWin->AddPointCloud(pG->m_name, &pG->m_tPC, &pG->m_mat);
				break;
			case pc_grid:
				string n = (pG->m_bStatic) ? ("static" + pG->m_name) : ("dynamic" + pG->m_name + i2str(pG->m_iGridLS));
				m_pWin->RemoveGeometry(n);
				m_pWin->AddLineSet(n, &pG->m_ls, &pG->m_mat);
				break;
			}
		}

		return true;
	}

	void _GeometryViewer::updateAllGeometries(void)
	{
		IF_(check() != OK_OK);
		IF_(!m_pWin);

		for (int i = 0; i < m_vGO.size(); i++)
		{
			GVIEWER_OBJ *pG = &m_vGO[i];
			IF_CONT(pG->m_bStatic);

			pG->updateGeometry();

			GEOMETRY_TYPE gt = pG->m_pGB->getType();
			switch (gt)
			{
			case pc_stream:
				m_pWin->UpdatePointCloud(pG->m_name, &pG->m_tPC);
				break;
			case pc_frame:
				m_pWin->UpdatePointCloud(pG->m_name, &pG->m_tPC);
				break;
			case pc_grid:
				m_pWin->UpdateLineSet("dynamic" + pG->m_name + i2str(pG->m_iGridLS), &pG->m_ls, &pG->m_mat);
				break;
			}
		}
	}

	void _GeometryViewer::updateCamProj(void)
	{
		IF_(check() != OK_OK);
		IF_(!m_pWin);

		if (m_camProj.m_type == 0) // Perspective
		{
			m_pWin->CamSetProj(m_camProj.m_fov,
							   m_camProj.m_vNF.x,
							   m_camProj.m_vNF.y,
							   m_camProj.m_fovType);
		}
		else
		{
			m_pWin->CamSetProj((Camera::Projection)m_camProj.m_type,
								m_camProj.m_vLR.x,
								m_camProj.m_vLR.y,
								m_camProj.m_vBT.x,
								m_camProj.m_vBT.y,
								m_camProj.m_vNF.x,
								m_camProj.m_vNF.y
								);
		}
	}

	void _GeometryViewer::updateCamPose(void)
	{
		IF_(check() != OK_OK);
		IF_(!m_pWin);

		m_pWin->CamSetPose(v2e(m_cam.m_vLookAt),
						   v2e(m_cam.m_vEye),
						   v2e(m_cam.m_vUp));
	}

	void _GeometryViewer::camBound(const AxisAlignedBoundingBox &aabb)
	{
		IF_(check() != OK_OK);
		IF_(!m_pWin);

		m_pWin->CamAutoBound(aabb, v2e(m_vCoR));
	}

	void _GeometryViewer::resetCamPose(void)
	{
		m_cam = m_camDefault;
	}

	void _GeometryViewer::setCamPose(const GVIEWER_CAM &camPose)
	{
		m_cam = camPose;
		updateCamPose();
	}

	GVIEWER_CAM _GeometryViewer::getCamPose(void)
	{
		return m_cam;
	}

	void _GeometryViewer::updateUI(void)
	{
		auto &app = gui::Application::GetInstance();
		app.Initialize(m_pathRes.c_str());

		m_pWin = new O3DUI(this->getName(), m_vWinSize.x, m_vWinSize.y);
		m_pUIstate = m_pWin->getUIState();
		m_pUIstate->m_bSceneCache = m_bSceneCache;
		m_pUIstate->m_mouseMode = (visualization::gui::SceneWidget::Controls)m_mouseMode;
		m_pUIstate->m_wPanel = m_wPanel;
		m_pUIstate->m_sMove = m_vDmove.x;
		m_pUIstate->m_dirSave = m_dirSave;
		m_pWin->Init();
		app.AddWindow(shared_ptr<O3DUI>(m_pWin));

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

	AxisAlignedBoundingBox _GeometryViewer::createDefaultAABB(void)
	{
		PointCloud pc;
		pc.points_.push_back(Vector3d(0, 0, 1));
		pc.points_.push_back(Vector3d(0, 0, -1));
		pc.points_.push_back(Vector3d(0, 1, 0));
		pc.points_.push_back(Vector3d(0, -1, 0));
		pc.points_.push_back(Vector3d(1, 0, 0));
		pc.points_.push_back(Vector3d(-1, 0, 0));
		return pc.GetAxisAlignedBoundingBox();
	}
}
