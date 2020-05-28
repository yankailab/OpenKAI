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
	m_vWinSize.init(1280,720);
}

_PointCloudViewer::~_PointCloudViewer()
{
}

bool _PointCloudViewer::init(void *pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	pK->v("vWinSize", &m_vWinSize);

	pK->v("fName", &m_fName);
	if(!m_fName.empty())
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

	if(m_pPC)
		*m_spPC = *m_pPC;

	m_vis.AddGeometry(m_spPC);
	m_bOpen = true;

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		m_vis.UpdateGeometry(m_spPC);
		m_vis.PollEvents();
		m_vis.UpdateRender();

		this->autoFPSto();
	}

	m_vis.DestroyVisualizerWindow();
}

int _PointCloudViewer::check(void)
{
	NULL__(m_pPC,-1);
	return 0;
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
