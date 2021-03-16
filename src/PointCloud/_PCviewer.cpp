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
}

_PCviewer::~_PCviewer()
{
}

bool _PCviewer::init(void *pKiss)
{
	IF_F(!this->_PCbase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	pK->v("vWinSize", &m_vWinSize);
	pK->v("fov", &m_fov);
	pK->v("bCoordFrame", &m_bCoordFrame);

	utility::SetVerbosityLevel(utility::VerbosityLevel::Error);

	//X:red, Y:green, Z:blue
    m_pMcoordFrame = open3d::geometry::TriangleMesh::CreateCoordinateFrame();

	string n;
	vector<string> vPCB;
	pK->a("vPCbase", &vPCB);
	IF_F(vPCB.empty());

	for(string p : vPCB)
	{
		_PCbase* pPCB = (_PCbase*) (pK->getInst(p));
		IF_CONT(!pPCB);

		m_vpPCB.push_back(pPCB);
	}
	IF_F(m_vpPCB.empty());

	return true;
}

bool _PCviewer::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

int _PCviewer::check(void)
{
	return this->_PCbase::check();
}

void _PCviewer::update(void)
{
	m_vis.CreateVisualizerWindow(*this->getName(), m_vWinSize.x, m_vWinSize.y);
	m_vis.GetRenderOption().background_color_ = Vector3d::Zero();
	m_vis.GetViewControl().ChangeFieldOfView(m_fov);

	m_vis.AddGeometry(m_spPC);

    if(m_bCoordFrame)
        m_vis.AddGeometry(m_pMcoordFrame);

	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		render();

		m_pT->autoFPSto();
	}

	m_vis.DestroyVisualizerWindow();
}

void _PCviewer::render(void)
{
	//read all inputs into one ring
	m_spPC->points_.clear();
	m_spPC->colors_.clear();
	for(_PCbase* pPCB : m_vpPCB)
	{
		pPCB->getRing()->get(&m_spPC->points_,
							 &m_spPC->colors_,
							m_pT->getTto());
	}

	m_vis.UpdateGeometry(m_spPC);

	if (m_vis.HasGeometry())
	{
		m_vis.PollEvents();
		m_vis.UpdateRender();
	}
}

void _PCviewer::draw(void)
{
	this->_PCbase::draw();
}

}
#endif
