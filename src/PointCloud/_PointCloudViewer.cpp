/*
 * _PointCloudViewer.cpp
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#include "_PointCloudViewer.h"

#ifdef USE_OPEN3D

namespace kai
{

_PointCloudViewer::_PointCloudViewer()
{
	m_type = pointCloud_viewer;
	m_vWinSize.init(1280, 720);
	m_pPC = NULL;

	pthread_mutex_init(&m_mutex, NULL);
}

_PointCloudViewer::~_PointCloudViewer()
{
	pthread_mutex_destroy(&m_mutex);
}

bool _PointCloudViewer::init(void *pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	pK->v("vWinSize", &m_vWinSize);

	pK->v("fName", &m_fName);
	if (!m_fName.empty())
	{
		m_pPC = new geometry::PointCloud();
		IF_F(!io::ReadPointCloud(m_fName.c_str(), *m_pPC));
	}

	utility::SetVerbosityLevel(utility::VerbosityLevel::Error);

	return true;
}

bool _PointCloudViewer::start(void)
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

void _PointCloudViewer::update(void)
{
	m_vis.CreateVisualizerWindow(this->getName()->c_str(), m_vWinSize.x, m_vWinSize.y);
	m_vis.GetRenderOption().background_color_ = Eigen::Vector3d::Zero();

	m_bOpen = true;

	if (m_pPC)
		*m_spPC = *m_pPC;

	m_vis.AddGeometry(m_spPC);

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		render();

		this->autoFPSto();
	}

	m_vis.DestroyVisualizerWindow();
}

int _PointCloudViewer::check(void)
{
	return 0;
}

void _PointCloudViewer::render(void)
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

void _PointCloudViewer::render(PointCloud* pPC)
{
	pthread_mutex_lock(&m_mutex);
	*m_spPC = *pPC;
	pthread_mutex_unlock(&m_mutex);
}

POINTCLOUD_TYPE _PointCloudViewer::getType(void)
{
	return m_type;
}

void _PointCloudViewer::draw(void)
{
	this->_ThreadBase::draw();
}

}
#endif
