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
		m_nPbuf = 0;

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
		IF_F(!this->_GeometryBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("nPbuf", &m_nPbuf);

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

		Kiss *pKt = pK->child("threadUI");
		IF_F(pKt->empty());
		m_pTui = new _Thread();
		if (!m_pTui->init(pKt))
		{
			DEL(m_pTui);
			return false;
		}

		if (m_nPbuf > 0)
		{
			m_PC.points_.reserve(m_nPbuf);
			m_PC.colors_.reserve(m_nPbuf);
		}

		return true;
	}

	bool _GeometryViewer::link(void)
	{
		IF_F(!this->_GeometryBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		vector<string> vGB;
		pK->a("vGeometryBase", &vGB);

		for (string p : vGB)
		{
			_GeometryBase *pGB = (_GeometryBase *)(pK->getInst(p));
			IF_CONT(!pGB);

			m_vpGB.push_back(pGB);
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
		m_pT->sleepT(USEC_1SEC);

		readAllGeometry();
		adjustNpoints(&m_PC, m_PC.points_.size(), m_nPbuf);
		removeUIpc(m_modelName);
		addUIpc(m_PC, m_modelName);

		m_pWin->AddGeometry(m_modelName + "staticLS", make_shared<geometry::LineSet>(m_staticLineSet));

		resetCamPose();
		updateCamPose();

		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			updateGeometry();
			updateDynamicLineSet(m_dynamicLineSet, m_modelName + "dynamicLS");

			m_pT->autoFPSto();
		}
	}

	void _GeometryViewer::updateGeometry(void)
	{
		IF_(check() < 0);

		readAllGeometry();
		// m_aabb = m_PC.GetAxisAlignedBoundingBox();
		// if (m_pUIstate)
		// 	m_pUIstate->m_sMove = m_vDmove.constrain(m_aabb.Volume() * 0.0001);

		adjustNpoints(&m_PC, m_PC.points_.size(), m_nPbuf);
		updateUIpc(m_PC, m_modelName);
	}

	void _GeometryViewer::readAllGeometry(void)
	{
		for (_GeometryBase *pGB : m_vpGB)
		{
			getGeometry(pGB);
		}
	}

	void _GeometryViewer::addUIpc(const PointCloud &pc, const string &name)
	{
		IF_(pc.IsEmpty());

		m_pWin->AddPointCloud(name,
							  make_shared<t::geometry::PointCloud>(
								  t::geometry::PointCloud::FromLegacy(
									  pc,
									  core::Dtype::Float32)));
	}

	void _GeometryViewer::updateUIpc(const PointCloud &pc, const string &name)
	{
		IF_(pc.IsEmpty());

		// TODO: atomic
		m_pWin->UpdatePointCloud(name,
								 make_shared<t::geometry::PointCloud>(
									 t::geometry::PointCloud::FromLegacy(
										 pc,
										 core::Dtype::Float32)));
	}

	void _GeometryViewer::removeUIpc(const string &name)
	{
		m_pWin->RemoveGeometry(name);
	}

	void _GeometryViewer::adjustNpoints(PointCloud *pPC, int nP, int nPbuf)
	{
		NULL_(pPC);

		if (nP < nPbuf)
		{
			addDummyPoints(pPC, nPbuf - nP, m_rDummyDome);
		}
		else if (nP > nPbuf)
		{
			int d = nP - nPbuf;
			pPC->points_.erase(pPC->points_.end() - d, pPC->points_.end());
			pPC->colors_.erase(pPC->colors_.end() - d, pPC->colors_.end());
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

	void _GeometryViewer::updateDynamicLineSet(const LineSet &ls, const string &name)
	{
		IF_(ls.IsEmpty());

		m_pWin->RemoveGeometry(name);

		Material mat;
		mat.SetBaseColor({1.0f, 1.0f, 1.0f, 1.0f});
		mat.SetMaterialName("defaultUnlit");
		mat.SetMaterialName("unlitLine");
		mat.SetLineWidth(10);

		m_pWin->AddGeometry(name,
							make_shared<geometry::LineSet>(ls),
							&mat);
	}

	void _GeometryViewer::getPCstream(void *p, const uint64_t &tExpire)
	{
		NULL_(p);
		_PCstream *pS = (_PCstream *)p;

		mutexLock();

		m_PC.Clear();

		uint64_t tNow = getApproxTbootUs();
		for (int i = 0; i < pS->nP(); i++)
		{
			GEOMETRY_POINT *pP = pS->get(i);
			if (tExpire)
			{
				IF_CONT(bExpired(pP->m_tStamp, tExpire, tNow));
			}

			m_PC.points_.push_back(pP->m_vP);
			m_PC.colors_.push_back(pP->m_vC.cast<double>());

			if (m_PC.points_.size() >= m_nPbuf)
				break;
		}

		mutexUnlock();
	}

	void _GeometryViewer::getPCframe(void *p)
	{
		NULL_(p);
		_PCframe *pF = (_PCframe *)p;

		m_PC = *pF->getBuffer();
	}

	void _GeometryViewer::getPCgrid(void *p)
	{
		NULL_(p);
		_PCgrid *pG = (_PCgrid *)p;

		m_staticLineSet = *pG->getGridLines();
		m_dynamicLineSet = *pG->getHLCLines();
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
