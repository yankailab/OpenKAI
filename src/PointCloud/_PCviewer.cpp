/*
 * _PCviewer.cpp
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifdef USE_OPEN3D
#include "_PCviewer.h"

namespace kai
{

	_PCviewer::_PCviewer()
	{
		m_fov = 0.0;
		m_vWinSize.init(1280, 720);
		m_pMcoordFrame = NULL;
		m_bCoordFrame = true;

		m_spPC = shared_ptr<PointCloud>(new PointCloud);
		m_pTgui = NULL;
	}

	_PCviewer::~_PCviewer()
	{
	}

	bool _PCviewer::init(void *pKiss)
	{
		IF_F(!this->_PCframe::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vWinSize", &m_vWinSize);
		pK->v("fov", &m_fov);
		pK->v("bCoordFrame", &m_bCoordFrame);

		utility::SetVerbosityLevel(utility::VerbosityLevel::Error);

		//X:red, Y:green, Z:blue
		m_pMcoordFrame = open3d::geometry::TriangleMesh::CreateCoordinateFrame();

		string n;
		vector<string> vPCB;
		pK->a("vPCbase", &vPCB);

		for (string p : vPCB)
		{
			_PCbase *pPCB = (_PCbase *)(pK->getInst(p));
			IF_CONT(!pPCB);

			m_vpPCB.push_back(pPCB);
		}

		Kiss *pKt = pK->child("thread");
		IF_F(pKt->empty());
		m_pTgui = new _Thread();
		if (!m_pTgui->init(pKt))
		{
			DEL(m_pTgui);
			return false;
		}

		return true;
	}

	bool _PCviewer::start(void)
	{
		NULL_F(m_pT);
		IF_F(!m_pT->start(getUpdate, this));
		
		NULL_F(m_pTgui);
		IF_F(!m_pTgui->start(getUpdateGUI, this));

		return true;
	}

	int _PCviewer::check(void)
	{
		return this->_PCbase::check();
	}

	void _PCviewer::update(void)
	{
		// m_vis.CreateVisualizerWindow(*this->getName(), m_vWinSize.x, m_vWinSize.y);
		// m_vis.GetRenderOption().background_color_ = Vector3d::Zero();
		// m_vis.GetViewControl().ChangeFieldOfView(m_fov);
		// m_vis.AddGeometry(m_spPC);
		// if (m_bCoordFrame)
		// 	m_vis.AddGeometry(m_pMcoordFrame);

		while(!m_spWin);
		
		render();

		vDouble3 vT,vR;
		vT.init(0.0);
		vR.init(0.0);

		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

//			render();
		for (_PCbase *pPCB : m_vpPCB)
		{
			readPC(pPCB);
		}
		updatePC();
		*m_spPC = *m_sPC.prev();

		vR.x += 0.1;
        Matrix4d mT = Matrix4d::Identity();
        Vector3d eR(vR.x,
                    vR.y,
                    vR.z);
        mT.block(0, 0, 3, 3) = Geometry3D::GetRotationMatrixFromXYZ(eR);
        mT(0, 3) = vT.x;
        mT(1, 3) = vT.y;
        mT(2, 3) = vT.z;
		m_spPC->Transform(mT);

		m_spWin->updateGeometry(m_spPC);

			m_pT->autoFPSto();
		}

//		m_vis.DestroyVisualizerWindow();
	}

	void _PCviewer::render(void)
	{
		//read all inputs into one
		for (_PCbase *pPCB : m_vpPCB)
		{
			readPC(pPCB);
		}

		updatePC();
		*m_spPC = *m_sPC.prev();

		m_spWin->SetGeometry(m_spPC, false);
		// m_vis.UpdateGeometry(m_spPC);

		// if (m_vis.HasGeometry())
		// {
		// 	m_vis.PollEvents();
		// 	m_vis.UpdateRender();
		// }
	}

	void _PCviewer::updateGUI(void)
	{
		auto &app = gui::Application::GetInstance();
		app.Initialize("/home/kai/dev/Open3D/build/bin/resources");

//		shared_ptr<GUIscan> win = shared_ptr<GUIscan>(new GUIscan("Open3D GUI", 2000, 1000));
		m_spWin = std::make_shared<GUIscan>("Open3D GUI", 2000, 1000);
		gui::Application::GetInstance().AddWindow(m_spWin);
		app.Run();
	}

	void _PCviewer::draw(void)
	{
		this->_PCframe::draw();
	}

}
#endif
