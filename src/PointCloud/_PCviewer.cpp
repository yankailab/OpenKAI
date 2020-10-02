/*
 * _PCviewer.cpp
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#include "_PCviewer.h"

#ifdef USE_OPEN3D

namespace kai
{

_PCviewer::_PCviewer()
{
	m_vWinSize.init(1280, 720);
	m_pMcoordFrame = NULL;
	m_fov = 0.0;
	m_spPC = shared_ptr<PointCloud>(new PointCloud);

	pthread_mutex_init(&m_mutex, NULL);
}

_PCviewer::~_PCviewer()
{
	pthread_mutex_destroy(&m_mutex);
}

bool _PCviewer::init(void *pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	pK->v("vWinSize", &m_vWinSize);
	pK->v("fov", &m_fov);

	m_pMcoordFrame = open3d::geometry::TriangleMesh::CreateCoordinateFrame();
	//X:red, Y:green, Z:blue

	utility::SetVerbosityLevel(utility::VerbosityLevel::Error);

	return true;
}

bool _PCviewer::start(void)
{
	IF_F(!this->_ThreadBase::start());

	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _PCviewer::update(void)
{
	m_vis.CreateVisualizerWindow(this->getName()->c_str(), m_vWinSize.x, m_vWinSize.y);
	m_vis.GetRenderOption().background_color_ = Eigen::Vector3d::Zero();
	m_vis.GetViewControl().ChangeFieldOfView(m_fov);

	m_vis.AddGeometry(m_spPC);
	m_vis.AddGeometry(m_pMcoordFrame);

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		render();

		this->autoFPSto();
	}

	m_vis.DestroyVisualizerWindow();
}

int _PCviewer::check(void)
{
	return 0;
}

void _PCviewer::render(void)
{
	pthread_mutex_lock(&m_mutex);
	m_vis.UpdateGeometry(m_spPC);
	pthread_mutex_unlock(&m_mutex);

	if (m_vis.HasGeometry())
	{
		m_vis.PollEvents();
		m_vis.UpdateRender();
	}
}

void _PCviewer::render(PointCloud* pPC)
{
	pthread_mutex_lock(&m_mutex);
	*m_spPC = *pPC;
	pthread_mutex_unlock(&m_mutex);
}

void _PCviewer::draw(void)
{
	this->_ThreadBase::draw();
}

}
#endif
