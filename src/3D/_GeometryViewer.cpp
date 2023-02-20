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

		m_pTui = NULL;
		m_pathRes = "";
		m_device = "CPU:0";
		m_vCoR.set(0, 0, 0);

		m_pWin = NULL;
		m_pUIstate = NULL;
		m_modelName = "PCMODEL";
		m_dirSave = "/home/lab/";

		m_bFullScreen = false;
		m_bSceneCache = false;
		m_wPanel = 15;
		m_mouseMode = 0;
		m_vDmove.set(0.5, 5.0);
		m_rDummyDome = 1000.0;
	}

	_GeometryViewer::~_GeometryViewer()
	{
		DEL(m_pWin);
	}

	bool _GeometryViewer::init(void *pKiss)
	{
		IF_F(!this->_PCframe::init(pKiss));
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
		pK->v("rDummyDome", &m_rDummyDome);

		pK->v("camFov", &m_camProj.m_fov);
		pK->v("vCamNF", &m_camProj.m_vNF);
		pK->v("camFovType", &m_camProj.m_fovType);

		pK->v("vCamLookAt", &m_camDefault.m_vLookAt);
		pK->v("vCamEye", &m_camDefault.m_vEye);
		pK->v("vCamUp", &m_camDefault.m_vUp);
		m_cam = m_camDefault;

		pK->v("vCamAutoLookAt", &m_camAuto.m_vLookAt);
		pK->v("vCamAutoEye", &m_camAuto.m_vEye);
		pK->v("vCamAutoUp", &m_camAuto.m_vUp);

		pK->v("vCoR", &m_vCoR);

		utility::SetVerbosityLevel(utility::VerbosityLevel::Error);

		string n;
		vector<string> vGB;
		pK->a("vGeometryBase", &vGB);

		for (string p : vGB)
		{
			_GeometryBase *pGB = (_GeometryBase *)(pK->getInst(p));
			IF_CONT(!pGB);

			m_vpGB.push_back(pGB);
		}

		Kiss *pKt = pK->child("threadUI");
		IF_F(pKt->empty());
		m_pTui = new _Thread();
		if (!m_pTui->init(pKt))
		{
			DEL(m_pTui);
			return false;
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

	int _GeometryViewer::check(void)
	{
		return this->_GeometryBase::check();
	}

	void _GeometryViewer::update(void)
	{
		// wait for the UI thread to get window ready
		// TODO: move to start()?
		m_pT->sleepT(0);

		while (m_nPread <= 0)
			readAllPC();

		if (m_nPread < m_nPresv)
		{
			addDummyPoints(m_sPC.next(), m_nPresv - m_nPread, m_rDummyDome, {0, 0, 0});
		}

		updatePC();
		removeUIpc();
		addUIpc(*m_sPC.get());

		resetCamPose();
		updateCamPose();

		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			updateGeometry();

			m_pT->autoFPSto();
		}
	}

	void _GeometryViewer::updateGeometry(void)
	{
		IF_(check() < 0);

		readAllPC();
		updatePC();
		PointCloud *pPC = m_sPC.get();
		pPC->normals_.clear();
		int n = pPC->points_.size();
		IF_(n <= 0);

		m_aabb = pPC->GetAxisAlignedBoundingBox();
		if (m_pUIstate)
			m_pUIstate->m_sMove = m_vDmove.constrain(m_aabb.Volume() * 0.0001);

		PointCloud pc = *pPC;
		if (n < m_nPresv)
		{
			addDummyPoints(&pc, m_nPresv - n, m_rDummyDome);
		}
		else if (n > m_nPresv)
		{
			int d = n - m_nPresv;
			pc.points_.erase(pc.points_.end() - d, pc.points_.end());
			pc.colors_.erase(pc.colors_.end() - d, pc.colors_.end());
		}

		updateUIpc(pc);
	}

	void _GeometryViewer::addUIpc(const PointCloud &pc)
	{
		IF_(pc.IsEmpty());

		m_pWin->AddPointCloud(m_modelName,
							  make_shared<t::geometry::PointCloud>(
								  t::geometry::PointCloud::FromLegacy(
									  pc,
									  core::Dtype::Float32)));
	}

	void _GeometryViewer::updateUIpc(const PointCloud &pc)
	{
		IF_(pc.IsEmpty());

		m_pWin->UpdatePointCloud(m_modelName,
								 make_shared<t::geometry::PointCloud>(
									 t::geometry::PointCloud::FromLegacy(
										 pc,
										 core::Dtype::Float32)));
	}

	void _GeometryViewer::removeUIpc(void)
	{
		m_pWin->RemoveGeometry(m_modelName);
	}

	void _GeometryViewer::readAllPC(void)
	{
		m_nPread = 0;
		for (_GeometryBase *pGB : m_vpGB)
		{
			readPC(pGB);
		}
	}

	void _GeometryViewer::addDummyPoints(PointCloud *pPC, int n, float r, Vector3d vCol)
	{
		NULL_(pPC);

		float nV = floor(sqrt((float)n));
		float nH = ceil(n / nV);

		float dV = OK_PI / nV;
		float dH = (OK_PI * 2.0) / nH;

		int k = 0;
		for (int i = 0; i < nH; i++)
		{
			float h = dH * i;
			float sinH = sin(h);
			float cosH = cos(h);

			for (int j = 0; j < nV; j++)
			{
				float v = dV * j;
				float sinV = sin(v);
				float cosV = cos(v);

				Vector3d vP(
					r * sinV * sinH,
					r * sinV * cosH,
					r * cosV);

				pPC->points_.push_back(vP);
				pPC->colors_.push_back(vCol);

				IF_(++k >= n);
			}
		}
	}

	void _GeometryViewer::updateCamProj(void)
	{
		IF_(check() < 0);
		IF_(!m_pWin);

		m_pWin->CamSetProj(m_camProj.m_fov,
						   m_camProj.m_vNF.x,
						   m_camProj.m_vNF.y,
						   m_camProj.m_fovType);
	}

	void _GeometryViewer::updateCamPose(void)
	{
		IF_(check() < 0);
		IF_(!m_pWin);

		m_pWin->CamSetPose(m_cam.m_vLookAt.v3f(),
						   m_cam.m_vEye.v3f(),
						   m_cam.m_vUp.v3f());
	}

	void _GeometryViewer::camBound(const AxisAlignedBoundingBox &aabb)
	{
		IF_(check() < 0);
		IF_(!m_pWin);

		m_pWin->CamAutoBound(aabb, m_vCoR.v3f());
	}

	void _GeometryViewer::resetCamPose(void)
	{
		m_cam = m_camDefault;
	}

	void _GeometryViewer::updateUI(void)
	{
		auto &app = gui::Application::GetInstance();
		app.Initialize(m_pathRes.c_str());

		m_pWin = new O3DUI(*this->getName(), 2000, 1000);
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

		m_pT->wakeUp();
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
