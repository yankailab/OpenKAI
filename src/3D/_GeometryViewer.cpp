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
		m_vBtnPadding.set(0);
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

		jKv<int>(j, "vWinSize", m_vWinSize);
		jKv(j, "pathRes", m_pathRes);
		jKv(j, "device", m_device);
		jKv(j, "dirSave", m_dirSave);

		jKv(j, "bFullScreen", m_bFullScreen);
		jKv(j, "bSceneCache", m_bSceneCache);
		jKv(j, "wPanel", m_wPanel);
		jKv<float>(j, "vBtnPadding", m_vBtnPadding);
		jKv(j, "mouseMode", m_mouseMode);
		jKv<float>(j, "vDmove", m_vDmove);

		jKv(j, "camProjType", m_camProj.m_type);
		jKv(j, "camFov", m_camProj.m_fov);
		jKv(j, "camFovType", m_camProj.m_fovType);
		jKv<float>(j, "vCamLR", m_camProj.m_vLR);
		jKv<float>(j, "vCamBT", m_camProj.m_vBT);
		jKv<float>(j, "vCamNF", m_camProj.m_vNF);

		jKv<float>(j, "vCamLookAt", m_camDefault.m_vLookAt);
		jKv<float>(j, "vCamEye", m_camDefault.m_vEye);
		jKv<float>(j, "vCamUp", m_camDefault.m_vUp);
		m_cam = m_camDefault;

		jKv<float>(j, "vCamAutoLookAt", m_camAuto.m_vLookAt);
		jKv<float>(j, "vCamAutoEye", m_camAuto.m_vEye);
		jKv<float>(j, "vCamAutoUp", m_camAuto.m_vUp);

		jKv<float>(j, "vCoR", m_vCoR);

		utility::SetVerbosityLevel(utility::VerbosityLevel::Error);

		DEL(m_pTui);
		m_pTui = createThread(jK(j, "threadUI"), "threadUI");
		NULL_F(m_pTui);

		return true;
	}

	bool _GeometryViewer::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_GeometryBase::link(j, pM));

		const json &jg = jK(j, "geometry");
		IF__(!jg.is_object(), true);

		for (auto it = jg.begin(); it != jg.end(); it++)
		{
			const json &Ji = it.value();
			IF_CONT(!Ji.is_object());

			string n = "";
			jKv(Ji, "_GeometryBase", n);
			_GeometryBase *pGB = (_GeometryBase *)(pM->findModule(n));
			IF_CONT(!pGB);

			GVIEWER_OBJ g;
			g.m_pGB = pGB;
			g.m_name = n;
			jKv(Ji, "bStatic", g.m_bStatic);
			jKv(Ji, "nP", g.m_nPbuf);
			jKv(Ji, "rDummyDome", g.m_rDummyDome);
			jKv(Ji, "matName", g.m_matName);
			jKv<float>(Ji, "matCol", g.m_matCol);
			jKv(Ji, "matPointSize", g.m_matPointSize);
			jKv(Ji, "matLineWidth", g.m_matLineWidth);
			jKv(Ji, "iGridLS", g.m_iGridLS);

			g.init();
			g.updateMaterial();

			m_vGO.push_back(g);
		}

		return true;
	}

	bool _GeometryViewer::start(void)
	{
		NULL_F(m_pT);
		IF_F(!m_pT->start(getUpdate, this));

		NULL_F(m_pTui);
		IF_F(!m_pTui->start(getUpdateUI, this));

		return true;
	}

	bool _GeometryViewer::check(void)
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
		IF_F(!check());
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
		IF_(!check());
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
		IF_(!check());
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
							   m_camProj.m_vNF.y);
		}
	}

	void _GeometryViewer::updateCamPose(void)
	{
		IF_(!check());
		IF_(!m_pWin);

		m_pWin->CamSetPose(v2e(m_cam.m_vLookAt),
						   v2e(m_cam.m_vEye),
						   v2e(m_cam.m_vUp));
	}

	void _GeometryViewer::camBound(const AxisAlignedBoundingBox &aabb)
	{
		IF_(!check());
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
