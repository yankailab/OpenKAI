/*
 * _PointCloudBase.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_PointCloudBase.h"

#ifdef USE_OPEN3D

namespace kai
{

_PointCloudBase::_PointCloudBase()
{
	m_bOpen = false;
	m_type = pointCloud_unknown;
	m_pPC = NULL;
	m_spPC = shared_ptr<PointCloud>(new PointCloud);
}

_PointCloudBase::~_PointCloudBase()
{
}

bool _PointCloudBase::init(void *pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	return true;
}

int _PointCloudBase::size(void)
{
	return -1;
}

POINTCLOUD_TYPE _PointCloudBase::getType(void)
{
	return m_type;
}

bool _PointCloudBase::open(void)
{
	return true;
}

bool _PointCloudBase::isOpened(void)
{
	return m_bOpen;
}

void _PointCloudBase::close(void)
{
	m_bOpen = false;
}

void _PointCloudBase::draw(void)
{
	this->_ThreadBase::draw();
}

}
#endif
